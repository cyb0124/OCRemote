#include <iostream>
#include <sstream>
#include "Factory.h"
#include "WeakCallback.h"

SharedPromise<std::monostate> Reservation::extract(size_t slot) const {
  std::vector<SharedPromise<std::monostate>> promises;
  for (auto &i : providers)
    promises.emplace_back(i.first->extract(i.second, slot));
  return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
}

void ItemInfo::addProvider(SharedProvider provider) {
  if (!provider->avail)
    return;
  nAvail += provider->avail;
  providers.emplace(std::move(provider));
}

void ItemInfo::backup(int size) {
  nBackup += size;
}

int ItemInfo::getAvail(bool allowBackup) const {
  if (allowBackup)
    return nAvail;
  else
    return std::max(0, nAvail - nBackup);
}

Reservation ItemInfo::reserve(int size) {
  Reservation result;
  while (size > 0) {
    auto &best(providers.top());
    int toProc(std::min(size, best->avail));
    result.providers.emplace_back(best, toProc);
    best->avail -= toProc;
    nAvail -= toProc;
    size -= toProc;
    if (best->avail <= 0)
      providers.pop();
  }
  return result;
}

void Factory::endOfCycle() {
  for (auto &i : storages)
    i->endOfCycle();
  items.clear();
  nameMap.clear();
  labelMap.clear();
  busEverUpdated = false;
  cycleDelayTimer = std::make_shared<boost::asio::steady_timer>(s.io.io);
  cycleDelayTimer->expires_at(cycleStartTime + std::chrono::milliseconds(minCycleTime));
  cycleDelayTimer->async_wait(makeWeakCallback(cycleDelayTimer, [this](const boost::system::error_code &ec) {
    if (ec)
      throw boost::system::system_error(ec);
    cycleDelayTimer.reset();
    start();
  }));
}

SharedPromise<std::monostate> Factory::updateAndBackupItems() {
  std::vector<SharedPromise<std::monostate>> promises;
  for (auto &i : storages)
    promises.emplace_back(i->update());
  return Promise<std::monostate>::all(promises)->map(alive, [this](auto&&) {
    int total{};
    for (auto &i : items)
      total += i.second.getAvail(true);
    log("storage: " + std::to_string(total) + " items, " + std::to_string(items.size()) + " types", 0x00ff00);
    for (auto &i : backups) {
      auto item(getItem(*i.first));
      if (!item)
        continue;
      items.at(item).backup(i.second);
    }
    return std::monostate{};
  });
}

void Factory::insertItem(std::vector<SharedPromise<std::monostate>> &promises, size_t slot, ItemStack stack) {
  while (stack.size > 0) {
    Storage *bestStorage{};
    int bestPriority;
    for (auto &i : storages) {
      auto nowPriority(i->sinkPriority(*stack.item));
      if (!nowPriority)
        continue;
      if (!bestStorage || *nowPriority > bestPriority) {
        bestStorage = i.get();
        bestPriority = *nowPriority;
      }
    }
    if (!bestStorage) {
      promises.emplace_back(scheduleFailingPromise<std::monostate>(s.io, "Storage is full"));
      break;
    }
    auto result{bestStorage->sink(stack, slot)};
    stack.size -= result.first;
    promises.emplace_back(std::move(result.second));
  }
}

void Factory::doBusUpdate() {
  busState = BusState::RUNNING;
  auto &access(s.getBestAccess(busAccesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.inv;
  action->side = access.sideBus;
  s.enqueueAction(access.client, action);

  struct TailListener : Listener<std::monostate> {
    Factory &rThis;
    std::weak_ptr<std::monostate> wk;
    TailListener(Factory &rThis) :rThis(rThis), wk(rThis.alive) {}

    void endOfBusUpdate() {
      rThis.busState = BusState::IDLE;
      if (rThis.endOfCycleAfterBusUpdate) {
        rThis.endOfCycleAfterBusUpdate = false;
        rThis.endOfCycle();
      }
    }

    void onFail(std::string cause) override {
      if (wk.expired())
        return;
      cause = "bus update failed: " + cause;
      rThis.log(cause, 0xff0000u, 880.f);
      for (auto &i : rThis.busWaitQueue)
        rThis.s.io([cont(std::move(i)), cause]() { cont->onFail(std::move(cause)); });
      rThis.busWaitQueue.clear();
      endOfBusUpdate();
    }
    void onResult(std::monostate) override {
      if (wk.expired())
        return;
      if (rThis.busState == BusState::RESTART) {
        rThis.doBusUpdate();
      } else {
        endOfBusUpdate();
      }
    }
  };

  action->then(alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::vector<size_t> freeSlots;
    for (size_t i{}; i < items.size(); ++i) {
      if (busAllocations.find(i) != busAllocations.end())
        continue;
      auto &stack(items[i]);
      if (stack) {
        log(stack->item->label + "*" + std::to_string(stack->size), 0xffa500);
        busState = BusState::RESTART;
        insertItem(promises, i, *stack);
      } else {
        freeSlots.emplace_back(i);
      }
    }
    while (!freeSlots.empty() && !busWaitQueue.empty()) {
      auto slot(freeSlots.back());
      freeSlots.pop_back();
      busAllocations.emplace(slot);
      s.io([cont(std::move(busWaitQueue.front())), slot]() { cont->onResult(slot); });
      busWaitQueue.pop_front();
    }
    if (promises.empty())
      return scheduleTrivialPromise(s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  })->listen(std::make_shared<TailListener>(*this));
}

void Factory::scheduleBusUpdate() {
  busEverUpdated = true;
  if (busState == BusState::IDLE)
    doBusUpdate();
  else
    busState = BusState::RESTART;
}

void Factory::log(std::string msg, uint32_t color, double beep) {
  std::cout << msg << std::endl;
  auto action(std::make_shared<Actions::Print>());
  action->text = std::move(msg);
  action->color = color;
  action->beep = beep;
  s.enqueueAction(logClient, action);
  action->listen(std::make_shared<DummyListener<std::monostate>>());
}

ItemInfo &Factory::getOrAddItemInfo(const SharedItem &item) {
  auto info(items.try_emplace(item));
  if (info.second) {
    nameMap.emplace(item->name, item);
    labelMap.emplace(item->label, item);
  }
  return info.first->second;
}

SharedItem Factory::getItem(const ItemFilters::Base &filter) {
  struct Visitor : ItemFilters::IndexVisitor {
    Factory &rThis;
    SharedItem result;
    Visitor(Factory &rThis) :rThis(rThis) {}

    void addResult(const SharedItem &item) {
      if (rThis.getAvail(item, true) > rThis.getAvail(result, true)) {
        result = item;
      }
    }

    void visit(const ItemFilters::Name &filter) override {
      for (auto i(rThis.nameMap.equal_range(filter.name)); i.first != i.second; ++i.first) {
        addResult(i.first->second);
      }
    }

    void visit(const ItemFilters::Label &filter) override {
      for (auto i(rThis.labelMap.equal_range(filter.label)); i.first != i.second; ++i.first) {
        addResult(i.first->second);
      }
    }

    void visit(const ItemFilters::LabelName &filter) override {
      for (auto i(rThis.labelMap.equal_range(filter.label)); i.first != i.second; ++i.first) {
        if (i.first->second->name == filter.name) {
          addResult(i.first->second);
        }
      }
    }

    void visit(const ItemFilters::Base &filter) override {
      for (auto &i : rThis.items) {
        if (filter.filter(*i.first)) {
          addResult(i.first);
        }
      }
    }
  } v(*this);
  filter.accept(v);
  return std::move(v.result);
}

int Factory::getAvail(const SharedItem &item, bool allowBackup) {
  if (!item)
    return 0;
  auto info(items.find(item));
  if (info == items.end())
    return 0;
  return info->second.getAvail(allowBackup);
}

Reservation Factory::reserve(const std::string &reason, const SharedItem &item, int size) {
  log(reason + ": " + item->label + "*" + std::to_string(size), 0x55abec);
  return items.at(item).reserve(size);
}

SharedPromise<size_t> Factory::busAllocate() {
  auto promise(std::make_shared<Promise<size_t>>());
  busWaitQueue.emplace_back(promise);
  scheduleBusUpdate();
  return promise;
}

void Factory::busFree(size_t slot) {
  busAllocations.erase(slot);
  scheduleBusUpdate();
}

void Factory::busFree(const std::vector<size_t> &slots) {
  for (size_t slot : slots)
    busAllocations.erase(slot);
  scheduleBusUpdate();
}

void Factory::start() {
  auto now(std::chrono::steady_clock::now());
  std::ostringstream os;
  os << "Cycle " << currentCycleNum << ", lastCycleTime=";
  os.precision(3);
  os << std::fixed << std::chrono::duration_cast<std::chrono::milliseconds>(now - cycleStartTime).count() * 1E-3f;
  log(os.str());
  cycleStartTime = now;

  struct TailListener : Listener<std::monostate> {
    Factory &rThis;
    std::weak_ptr<std::monostate> wk;
    explicit TailListener(Factory &rThis)
        :rThis(rThis), wk(rThis.alive) {}

    void onFail(std::string cause) override {
      if (wk.expired()) return;
      rThis.log("Cycle failed: " + cause, 0xff0000u, 880.f);
      if (rThis.busState == BusState::IDLE)
        rThis.endOfCycle();
      else
        rThis.endOfCycleAfterBusUpdate = true;
    }

    void onResult(std::monostate result) override {
      if (wk.expired()) return;
      ++rThis.currentCycleNum;
      if (rThis.busState == BusState::IDLE) {
        if (rThis.busEverUpdated) {
          rThis.endOfCycle();
        } else {
          rThis.endOfCycleAfterBusUpdate = true;
          rThis.scheduleBusUpdate();
        }
      } else {
        rThis.endOfCycleAfterBusUpdate = true;
      }
    }
  };

  updateAndBackupItems()->then(alive, [this](std::monostate) {
    std::vector<SharedPromise<std::monostate>> promises;
    for (auto &i : processes)
      promises.emplace_back(i->cycle());
    if (promises.empty())
      return scheduleTrivialPromise(s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  })->listen(std::make_shared<TailListener>(*this));
}
