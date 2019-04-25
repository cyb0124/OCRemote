#include <iostream>
#include "Factory.h"
#include "WeakCallback.h"

void Factory::log(std::string msg, uint32_t color, std::optional<float> beep) {
  struct DummyListener : Listener<std::monostate> {
    void onFail(std::string cause) override {}
    void onResult(std::monostate result) override {}
  };

  std::cout << msg << std::endl;
  auto action(std::make_shared<Actions::Print>());
  action->text = std::move(msg);
  action->color = color;
  action->beep = beep;
  s.enqueueAction(baseClient, action);
  action->listen(std::make_shared<DummyListener>());
}

void Factory::addChest(const XNetCoord &pos) {
  chests.emplace_back(pos);
}

void Factory::cycle() {
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

  cycleStartTime = std::chrono::steady_clock::now();
  log("Cycle " + std::to_string(currentCycleNum));
  gatherChests()->listen(std::make_shared<ResultListener>(*this));
}

void Factory::endOfCycle() {
  itemProviders.clear();
  nameMap.clear();
  labelMap.clear();

  cycleDelayTimer = std::make_shared<boost::asio::steady_timer>(s.io.io);
  cycleDelayTimer->expires_at(cycleStartTime + std::chrono::milliseconds(minCycleTime));
  cycleDelayTimer->async_wait(makeWeakCallback(cycleDelayTimer, [this](const boost::system::error_code &e) {
    if (e) throw boost::system::system_error(e);
    cycleDelayTimer.reset();
    cycle();
  }));
}

SharedPromise<std::monostate> Factory::gatherChest(const XNetCoord &pos) {
  auto action(std::make_shared<Actions::ListXN>());
  action->inv = baseInv;
  action->side = -1;
  action->pos = pos - basePos;
  s.enqueueAction(baseClient, action);
  return action->map([this, pos, wk(std::weak_ptr(alive))](std::vector<SharedItemStack> items) -> std::monostate {
    if (wk.expired()) return {};
    for (size_t i = 0; i < items.size(); ++i) {
      auto &item(items[i]);
      if (!item) continue;
      nameMap.emplace(item->item->name, itemProviders.size());
      labelMap.emplace(item->item->label, itemProviders.size());
      auto &provider = itemProviders.emplace_back();
      provider.pos = pos;
      provider.slot = i + 1;
      provider.item = std::move(item);
    }
    return {};
  });
}

SharedPromise<std::monostate> Factory::gatherChests() {
  std::vector<SharedPromise<std::monostate>> promises;
  for (auto &i : chests)
    promises.emplace_back(gatherChest(i));
  return Promise<std::monostate>::all(promises)->map([this, wk(std::weak_ptr(alive))](auto) -> std::monostate {
    if (wk.expired()) return {};
    log("Gathered " + std::to_string(itemProviders.size()) + " slots");
    return {};
  });
}
