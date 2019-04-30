#include <iostream>
#include <sstream>
#include "Factory.h"
#include "WeakCallback.h"

void ItemInfo::addProvider(const XNetCoord &pos, int slot, int size) {
  if (!size) throw std::logic_error("empty provider");
  auto &provider = providers.emplace_back();
  provider.pos = pos;
  provider.slot = slot;
  provider.size = size;
  nAvail += size;
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

void Factory::log(std::string msg, uint32_t color, float beep) {
  std::cout << msg << std::endl;
  auto action(std::make_shared<Actions::Print>());
  action->text = std::move(msg);
  action->color = color;
  action->beep = beep;
  s.enqueueAction(baseClient, action);
  action->listen(std::make_shared<DummyListener<std::monostate>>());
}

SharedItem Factory::getItem(const ItemFilters::Base &filter) {
  struct Visitor : ItemFilters::IndexVisitor {
    Factory &rThis;
    SharedItem result;
    explicit Visitor(Factory &rThis) :rThis(rThis) {}

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
  const SharedItem &item, int size,
  const XNetCoord &to, int side
) {
  log(reason + ": " + item->label + "*" + std::to_string(size), 0x55abec);
  auto &info(items.at(item));
  while (size) {
    auto fulfilled(info.extractSome(size));
    auto fromPos(fulfilled.pos - basePos);
    auto toPos(to - basePos);
    auto action(std::make_shared<Actions::Call>());
    action->inv = baseInv;
    action->fn = "transferItem";
    action->args.push_back({{"x", fromPos.x}, {"y", fromPos.y}, {"z", fromPos.z}});
    action->args.push_back(fulfilled.slot);
    action->args.push_back(fulfilled.size);
    action->args.push_back({{"x", toPos.x}, {"y", toPos.y}, {"z", toPos.z}});
    action->args.push_back(Actions::bottom);
    if (side >= 0) action->args.push_back(side);
    promises.emplace_back(action->map([](auto) -> std::monostate { return {}; }));
    s.enqueueAction(baseClient, std::move(action));
    size -= fulfilled.size;
  }
}

ItemProvider ItemInfo::extractSome(int size) {
  auto best = std::min_element(providers.begin(), providers.end(), [](const ItemProvider &x, const ItemProvider &y) {
    return x.size < y.size;
  });
  ItemProvider result(*best);
  result.size = std::min(result.size, size);
  nAvail -= result.size;
  if (!(best->size -= result.size))
    providers.erase(best);
  return result;
}

void Factory::addChest(const XNetCoord &pos) {
  chests.emplace_back(pos);
}

void Factory::addBackup(SharedItemFilter filter, int size) {
  backups.emplace_back(std::move(filter), size);
}

void Factory::addProcess(SharedProcess process) {
  processes.emplace_back(std::move(process));
}

void Factory::start() {
  auto now(std::chrono::steady_clock::now());
  std::ostringstream os;
  os << "Cycle " << currentCycleNum << ", lastCycleTime=";
  os.precision(3);
  os << std::fixed << std::chrono::duration_cast<std::chrono::milliseconds>(now - cycleStartTime).count() * 1E-3f;
  log(os.str());
  cycleStartTime = now;

  struct ResultListener : Listener<std::monostate> {
    Factory &rThis;
    std::weak_ptr<std::monostate> wk;
    explicit ResultListener(Factory &rThis)
        :rThis(rThis), wk(rThis.alive) {}

    void onFail(std::string cause) override {
      if (wk.expired()) return;
      rThis.log("Cycle failed: " + cause, 0xff0000u, 880.f);
      rThis.endOfCycle();
    }

    void onResult(std::monostate result) override {
      if (wk.expired()) return;
      rThis.endOfCycle();
      ++rThis.currentCycleNum;
    }
  };

  updateAndBackupItems()->then([wk(std::weak_ptr(alive)), &io(s.io), this](std::monostate) {
    std::vector<SharedPromise<std::monostate>> promises;
    if (!wk.expired())
      for (auto &i : processes)
        promises.emplace_back(i->cycle(*this));
    if (promises.empty())
      return makeEmptyPromise(io);
    return Promise<std::monostate>::all(promises)->map([](auto) -> std::monostate { return {}; });
  })->listen(std::make_shared<ResultListener>(*this));
}

void Factory::endOfCycle() {
  items.clear();
  nameMap.clear();
  labelMap.clear();

  cycleDelayTimer = std::make_shared<boost::asio::steady_timer>(s.io.io);
  cycleDelayTimer->expires_at(cycleStartTime + std::chrono::milliseconds(minCycleTime));
  cycleDelayTimer->async_wait(makeWeakCallback(cycleDelayTimer, [this](const boost::system::error_code &e) {
    if (e) throw boost::system::system_error(e);
    cycleDelayTimer.reset();
    start();
  }));
}

ItemInfo &Factory::getOrAddInfo(const SharedItem &item) {
  auto info(items.try_emplace(item));
  if (info.second) {
    nameMap.emplace(item->name, item);
    labelMap.emplace(item->label, item);
  }
  return info.first->second;
}

SharedPromise<std::monostate> Factory::updateChest(const XNetCoord &pos) {
  auto action(std::make_shared<Actions::ListXN>());
  action->inv = baseInv;
  action->side = Actions::bottom;
  action->pos = pos - basePos;
  s.enqueueAction(baseClient, action);
  return action->map([this, pos, wk(std::weak_ptr(alive))](std::vector<SharedItemStack> xs) -> std::monostate {
    if (wk.expired()) return {};
    for (size_t i = 0; i < xs.size(); ++i) {
      auto &x(xs[i]);
      if (!x) continue;
      getOrAddInfo(x->item).addProvider(pos, i + 1, x->size);
    }
    return {};
  });
}

SharedPromise<std::monostate> Factory::updateAndBackupItems() {
  std::vector<SharedPromise<std::monostate>> promises;
  for (auto &i : chests) {
    promises.emplace_back(updateChest(i));
  }

  return Promise<std::monostate>::all(promises)->map([this, wk(std::weak_ptr(alive))](auto) -> std::monostate {
    if (wk.expired()) return {};
    for (auto &i : backups) {
      auto item(getItem(*i.first));
      if (!item) continue;
      items.at(item).backup(i.second);
    }
    return {};
  });
}
