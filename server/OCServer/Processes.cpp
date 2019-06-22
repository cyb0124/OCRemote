#include "Processes.h"

SharedPromise<std::monostate> ProcessSingleBlock::processOutput(int slot, int size) {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(sideCrafter),
    static_cast<double>(sideBus),
    static_cast<double>(size),
    static_cast<double>(slot),
  };
  return factory.busAllocate()->then([
    &io(factory.s.io), wk(std::weak_ptr(factory.alive)), this, action(std::move(action))
  ](size_t busSlot) {
    if (wk.expired())
      return makeEmptyPromise<std::monostate>(io);
    action->args.push_back(static_cast<double>(busSlot));
    factory.s.enqueueAction(client, action);
    return action->map([](auto&&) { return std::monostate{}; })->finally([wk, busSlot, this]() {
      if (!wk.expired())
        factory.busFree(busSlot);
    });
  });
}

SharedPromise<std::monostate> ProcessSlotted::cycle() {
  if (!outFilter)
    if (factory.getDemand(recipes).empty())
      return makeEmptyPromise<std::monostate>(factory.s.io);
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then([&io(factory.s.io), wk(std::weak_ptr(factory.alive)), this](std::vector<SharedItemStack> &&items) {
    if (wk.expired())
      return makeEmptyPromise<std::monostate>(io);
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<size_t, SharedItemStack> slotInfos;
    for (size_t slot : inSlots)
      slotInfos.try_emplace(slot);
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto stack(items[slot]);
      if (!stack)
        continue;
      auto info(slotInfos.find(slot));
      if (info == slotInfos.end()) {
        if (outFilter && outFilter(slot, *stack))
          promises.emplace_back(processOutput(slot, stack->item->maxSize));
      } else {
        info->second = stack;
      }
    }
    auto demands(factory.getDemand(recipes));
    for (auto &demand : demands) {
      auto &recipe(demand.recipe);
      int sets{demand.inAvail};
      std::unordered_set<size_t> usedSlots;
      std::vector<int> eachSize;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        auto &in(recipe.in[i]);
        eachSize.emplace_back(in.size / static_cast<int>(in.data.size()));
        for (size_t slot : in.data) {
          auto &info(slotInfos.at(slot));
          if (info && *info->item != *demand.in[i]) {
            goto skip;
          } else {
            usedSlots.emplace(slot);
            int inProc{info ? info->size : 0};
            sets = std::min(sets, (recipe.data - inProc) / recipe.data);
            if (sets <= 0)
              goto skip;
          }
        }
      }
      for (auto &info : slotInfos) {
        if (info.second && usedSlots.find(info.first) == usedSlots.end())
          goto skip;
      }
      auto slotsToFree(std::make_shared<std::vector<size_t>>());
      std::vector<SharedPromise<size_t>> extractions;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        extractions.emplace_back(factory.busAllocate()->then([
          &io, wk, this, slotsToFree, reservation(factory.reserve(name, demand.in[i], sets * recipe.in[i].size))
        ](size_t busSlot) {
          if (wk.expired())
            return makeEmptyPromise<size_t>(io);
          slotsToFree->emplace_back(busSlot);
          return reservation.extract(busSlot)->map([busSlot](auto&&) { return busSlot; });
        }));
      }
      promises.emplace_back(Promise<size_t>::all(extractions)->then([
        &io, wk, this, sets, eachSize(std::move(eachSize)), &recipe
      ](std::vector<size_t> &&busSlots) {
        if (wk.expired())
          return makeEmptyPromise<std::monostate>(io);
        std::vector<SharedPromise<std::monostate>> promises;
        std::vector<SharedAction> actions;
        for (size_t i{}; i < recipe.in.size(); ++i) {
          for (size_t toSlot : recipe.in[i].data) {
            auto action(std::make_shared<Actions::Call>());
            action->inv = inv;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(sideBus),
              static_cast<double>(sideCrafter),
              static_cast<double>(sets * eachSize[i]),
              static_cast<double>(busSlots[i]),
              static_cast<double>(toSlot)
            };
            promises.emplace_back(action->map([](auto&&) { return std::monostate{}; }));
            actions.emplace_back(std::move(action));
          }
        }
        factory.s.enqueueActionGroup(client, std::move(actions));
        return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
      })->finally([wk, this, slotsToFree(std::move(slotsToFree))]() {
        if (!wk.expired())
          factory.busFree(*slotsToFree);
      }));
      break;
      skip:;
    }
    if (promises.empty())
      return makeEmptyPromise<std::monostate>(io);
    return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
  });
}
