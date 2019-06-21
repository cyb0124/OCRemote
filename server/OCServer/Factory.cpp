#include <iostream>
#include <sstream>
#include "Factory.h"
#include "WeakCallback.h"

void ItemInfo::addProvider(UniqueProvider provider) {
  if (!provider->size)
    return;
  nAvail += provider->size;
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

void ItemInfo::extract(std::vector<SharedPromise<std::monostate>> &promises, int size, int slot) {
  while (size >= 0) {
    auto &best(*providers.top());
    int toProc(std::min(size, best.size));
    promises.emplace_back(best.extract(toProc, slot));
    size -= toProc;
    if (best.size)
      providers.pop();
  }
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
  return Promise<std::monostate>::all(promises)->map([this, wk(std::weak_ptr(alive))](auto&&) -> std::monostate {
    if (wk.expired()) return {};
    for (auto &i : backups) {
      auto item(getItem(*i.first));
      if (!item)
        continue;
      items.at(item).backup(i.second);
    }
    return {};
  });
}

void Factory::insertItem(std::vector<SharedPromise<std::monostate>> &promises, size_t slot, ItemStack stack) {
  while (stack.size >= 0) {
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
    if (!bestStorage)
      break;
    auto result{bestStorage->sink(stack, slot)};
    stack.size -= result.first;
    promises.emplace_back(std::move(result.second));
  }
}

void Factory::doBusUpdate() {
  busState = BusState::RUNNING;
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideBus;
  s.enqueueAction(client, action);

  struct TailListener : Listener<std::monostate> {
    Factory &rThis;
    std::weak_ptr<std::monostate> wk;
    TailListener(Factory &rThis) :rThis(rThis), wk(rThis.alive) {}
    void onFail(std::string cause) override {
      if (wk.expired())
        return;
      rThis.log("Bus update failed: " + cause, 0xff0000u, 880.f);
      rThis.doBusUpdate();
    }
    void onResult(std::monostate) override {
      if (wk.expired())
        return;
      if (rThis.busState == BusState::RESTART) {
        rThis.doBusUpdate();
      } else {
        rThis.busState = BusState::IDLE;
        if (rThis.endOfCycleAfterBusUpdate) {
          rThis.endOfCycleAfterBusUpdate = false;
          rThis.endOfCycle();
        }
      }
    }
  };

  action->then([&io(s.io), this, wk(std::weak_ptr(alive))](std::vector<SharedItemStack> &&items) {
    if (wk.expired())
      return makeEmptyPromise(io);
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
    for (auto i{busWaitQueue.begin()}; !freeSlots.empty() && i != busWaitQueue.end();) {
      if (i->n <= freeSlots.size() || i->allowPartial) {
        size_t toProc{std::min(i->n, freeSlots.size())};
        std::vector<size_t> allocated;
        allocated.reserve(toProc);
        for (size_t i{}; i < toProc; ++i) {
          auto slot(freeSlots.back());
          freeSlots.pop_back();
          allocated.push_back(slot);
          busAllocations.emplace(slot);
        }
        s.io([cont(std::move(i->cont)), result(std::move(allocated))]() {
          cont->onResult(std::move(result));
        });
        i = busWaitQueue.erase(i);
      } else {
        ++i;
      }
    }
    if (promises.empty())
      return makeEmptyPromise(io);
    return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
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
  s.enqueueAction(client, action);
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
  if (!item) return 0;
  return items.at(item).getAvail(allowBackup);
}

void Factory::extract(
  std::vector<SharedPromise<std::monostate>> &promises, const std::string &reason,
  const SharedItem &item, int size, int slot
) {
  log(reason + ": " + item->label + "*" + std::to_string(size), 0x55abec);
  items.at(item).extract(promises, size, slot);
}

SharedPromise<std::vector<size_t>> Factory::busAllocate(bool allowPartial, size_t n) {
  auto &node{busWaitQueue.emplace_back()};
  node.cont = std::make_shared<Promise<std::vector<size_t>>>();
  node.allowPartial = allowPartial;
  node.n = n;
  scheduleBusUpdate();
  return node.cont;
}

void Factory::busFree(const std::vector<size_t> &slots) {
  for (size_t i : slots)
    busAllocations.erase(i);
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

  updateAndBackupItems()->then([wk(std::weak_ptr(alive)), &io(s.io), this](std::monostate) {
    std::vector<SharedPromise<std::monostate>> promises;
    if (!wk.expired())
      for (auto &i : processes)
        promises.emplace_back(i->cycle());
    if (promises.empty())
      return makeEmptyPromise(io);
    return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
  })->listen(std::make_shared<TailListener>(*this));
}
