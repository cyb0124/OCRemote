#include "Processes.h"

SharedPromise<std::monostate> ProcessSingleBlock::processOutput(int slot, int size) {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(sideCrafter),
    static_cast<double>(sideBus),
    static_cast<double>(size),
    static_cast<double>(slot)
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

SharedPromise<std::monostate> ProcessWorkingSet::cycle() {
  if (!outFilter)
    if (factory.getDemand(recipes, false).empty())
      return makeEmptyPromise<std::monostate>(factory.s.io);
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then([&io(factory.s.io), wk(std::weak_ptr(factory.alive)), this](std::vector<SharedItemStack> &&items) {
    if (wk.expired())
      return makeEmptyPromise<std::monostate>(io);
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<SharedItem, int, SharedItemHash, SharedItemEqual> inProcMap;
    auto demands(factory.getDemand(recipes, false));
    for (auto &i : demands)
      inProcMap.try_emplace(i.in.front(), 0);
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (!stack)
        continue;
      auto itr(inProcMap.find(stack->item));
      if (itr == inProcMap.end()) {
        if (outFilter) {
          bool notInput{true};
          for (auto &recipe : recipes) {
            if (recipe.in.front().item->filter(*stack->item)) {
              notInput = false;
              break;
            }
          }
          if (notInput && outFilter(slot, *stack))
            promises.emplace_back(processOutput(slot, stack->item->maxSize));
        }
      } else {
        itr->second += stack->size;
      }
    }
    for (auto &demand : demands) {
      auto &recipe(demand.recipe);
      int toProc{std::min({
        recipe.data.second - inProcMap.at(demand.in.front()),
        factory.getAvail(demand.in.front(), recipe.in.front().allowBackup),
        demand.in.front()->maxSize})};
      if (toProc <= 0)
        continue;
      promises.emplace_back(factory.busAllocate()->then([
        &io, wk, this, toProc, reservation(factory.reserve(recipe.data.first, demand.in.front(), toProc))
      ](size_t busSlot) {
        if (wk.expired())
          return makeEmptyPromise<std::monostate>(io);
        return reservation.extract(busSlot)->then([
          &io, wk, this, toProc, busSlot
        ](auto&&) {
          if (wk.expired())
            return makeEmptyPromise<std::monostate>(io);
          auto action(std::make_shared<Actions::Call>());
          action->inv = inv;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(sideBus),
            static_cast<double>(sideCrafter),
            static_cast<double>(toProc),
            static_cast<double>(busSlot)
          };
          factory.s.enqueueAction(client, action);
          return action->map([](auto&&) { return std::monostate{}; });
        })->finally([wk, this, busSlot]() {
          if (!wk.expired())
            factory.busFree(busSlot);
        });
      }));
    }
    if (promises.empty())
      return makeEmptyPromise<std::monostate>(io);
    return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
  });
}

namespace {
  struct StockInfo : StockEntry {
    std::optional<size_t> slot;
    SharedItem resolved;
    StockInfo(const StockEntry &from) :StockEntry(from) {}
  };
}

SharedPromise<std::monostate> ProcessHeterogeneousWorkingSet::cycle() {
  if (!outFilter && stockList.empty())
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
    std::vector<StockInfo> stockInfos(stockList.begin(), stockList.end());
    auto recipeInProc(this->recipeMaxInProc);
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (!stack)
        continue;
      for (auto &stockInfo : stockInfos) {
        if (stockInfo.item->filter(*stack->item)) {
          stockInfo.resolved = stack->item;
          stockInfo.slot = slot;
          stockInfo.toStock -= stack->size;
          goto isStockItem;
        }
      }
      recipeMaxInProc -= stack->size;
      if (outFilter && outFilter(slot, *stack))
        promises.emplace_back(processOutput(slot, stack->item->maxSize));
      isStockItem:;
    }
    for (auto &stockInfo : stockInfos) {
      auto stockItem(stockInfo.resolved ? stockInfo.resolved : factory.getItem(*stockInfo.item));
      if (!stockItem)
        continue;
      int toProc{std::min({stockInfo.toStock, stockItem->maxSize, factory.getAvail(stockItem, stockInfo.allowBackup)})};
      if (toProc <= 0)
        continue;
      promises.emplace_back(factory.busAllocate()->then([
        &io, wk, this, toProc, reservation(factory.reserve(name, stockItem, toProc)), toSlot(stockInfo.slot)
      ](size_t busSlot) {
        if (wk.expired())
          return makeEmptyPromise<std::monostate>(io);
        return reservation.extract(busSlot)->then([
          &io, wk, this, toProc, busSlot
        ](auto&&) {
          if (wk.expired())
            return makeEmptyPromise<std::monostate>(io);
          auto action(std::make_shared<Actions::Call>());
          action->inv = inv;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(sideBus),
            static_cast<double>(sideCrafter),
            static_cast<double>(toProc),
            static_cast<double>(busSlot)
          };
          if (toSlot.has_value())
            action->args.emplace_back(static_cast<double>(*toSlot));
          factory.s.enqueueAction(client, action);
          return action->map([](auto&&) { return std::monostate{}; });
        })->finally([wk, this, busSlot]() {
          if (!wk.expired())
            factory.busFree(busSlot);
        });
      }));
    }
    if (recipeMaxInProc > 0) {
      auto demands(factory.getDemand(recipes));
      for (auto &demand : demands) {
        auto &recipe(demand.recipe);
        demand.in.clear();
        factory.resolveRecipeInputs(recipe, demand, true);
        int listSum{};
        for (auto &ingredient : recipe.in)
          listSum += ingredient.size;
        int sets{std::min(demand.inAvail, recipeMaxInProc / listSum)};
        if (sets <= 0)
          continue;
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
          &io, wk, this, sets, &recipe
        ](std::vector<size_t> &&busSlots) {
          if (wk.expired())
            return makeEmptyPromise<std::monostate>(io);
          std::vector<SharedPromise<std::monostate>> promises;
          std::vector<SharedAction> actions;
          for (size_t i{}; i < recipe.in.size(); ++i) {
            auto action(std::make_shared<Actions::Call>());
            action->inv = inv;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(sideBus),
              static_cast<double>(sideCrafter),
              static_cast<double>(sets * recipe.in[i].size),
              static_cast<double>(busSlots[i])
            };
            promises.emplace_back(action->map([](auto&&) { return std::monostate{}; }));
            actions.emplace_back(std::move(action));
          }
          factory.s.enqueueActionGroup(client, std::move(actions));
          return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
        })->finally([wk, this, slotsToFree(std::move(slotsToFree))]() {
          if (!wk.expired())
            factory.busFree(*slotsToFree);
        }));
        recipeMaxInProc -= sets * listSum;
        if (recipeMaxInProc <= 0)
          break;
      }
    }
    if (promises.empty())
      return makeEmptyPromise<std::monostate>(io);
    return Promise<std::monostate>::all(promises)->map([](auto&&) { return std::monostate{}; });
  });
}

SharedPromise<std::monostate> ProcessInputless::cycle() {
  auto needed(this->needed());
  if (needed <= 0)
    return makeEmptyPromise<std::monostate>(factory.s.io);
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then([&io(factory.s.io), wk(std::weak_ptr(factory.alive)), this, needed](std::vector<SharedItemStack> &&items) {
    if (wk.expired() || sourceSlot >= items.size() || !items[sourceSlot])
      return makeEmptyPromise<std::monostate>(io);
    int toProc{std::min(needed, items[sourceSlot]->size)};
    if (toProc <= 0)
      return makeEmptyPromise<std::monostate>(io);
    return processOutput(sourceSlot, toProc);
  });
}
