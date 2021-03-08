#include <set>
#include <iomanip>
#include "Processes.h"

SharedPromise<std::monostate> ProcessAccessInv::processOutput(size_t slot, int size) {
  return factory.busAllocate()->then(factory.alive, [this, slot, size](size_t busSlot) {
    auto &access(factory.s.getBestAccess(accesses));
    auto action(std::make_shared<Actions::Call>());
    action->inv = access.addr;
    action->fn = "transferItem";
    action->args = {
      static_cast<double>(access.sideInv),
      static_cast<double>(access.sideBus),
      static_cast<double>(size),
      static_cast<double>(slot + 1),
      static_cast<double>(busSlot + 1)
    };
    factory.s.enqueueAction(access.client, action);
    return action->mapTo(std::monostate{})->finally(factory.alive, [busSlot, this]() {
      factory.busFree(busSlot, true);
    });
  });
}

SharedPromise<std::monostate> ProcessSlotted::cycle() {
  if (!outFilter)
    if (factory.getDemand(recipes).empty())
      return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
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
      auto &recipe(*demand.recipe);
      int sets{demand.inAvail};
      std::unordered_set<size_t> usedSlots;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        auto &in(recipe.in[i]);
        auto eachSize(in.size / static_cast<int>(in.data.size()));
        for (size_t slot : in.data) {
          auto &info(slotInfos.at(slot));
          if (info && *info->item != *demand.in[i]) {
            goto skip;
          } else {
            usedSlots.emplace(slot);
            int inProc{info ? info->size : 0};
            sets = std::min(sets, (std::min(recipe.data, demand.in[i]->maxSize) - inProc) / eachSize);
            if (sets <= 0)
              goto skip;
          }
        }
      }
      for (auto &info : slotInfos) {
        if (info.second && usedSlots.find(info.first) == usedSlots.end()) {
          goto skip;
        }
      }
      {
        auto slotsToFree(std::make_shared<std::vector<size_t>>());
        std::vector<SharedPromise<size_t>> extractions;
        for (size_t i{}; i < recipe.in.size(); ++i) {
          extractions.emplace_back(factory.busAllocate()->then(factory.alive, [
            this, slotsToFree, reservation(factory.reserve(name, demand.in[i], sets * recipe.in[i].size))
          ](size_t busSlot) {
            slotsToFree->emplace_back(busSlot);
            return reservation.extract(busSlot)->mapTo(busSlot);
          }));
        }
        promises.emplace_back(Promise<size_t>::all(extractions)->then(factory.alive, [this, sets, &recipe](std::vector<size_t> &&busSlots) {
          auto &access(factory.s.getBestAccess(accesses));
          std::vector<SharedPromise<std::monostate>> promises;
          std::vector<SharedAction> actions;
          for (size_t i{}; i < recipe.in.size(); ++i) {
            auto &in(recipe.in[i]);
            auto eachSize(in.size / static_cast<int>(in.data.size()));
            for (size_t toSlot : in.data) {
              auto action(std::make_shared<Actions::Call>());
              action->inv = access.addr;
              action->fn = "transferItem";
              action->args = {
                static_cast<double>(access.sideBus),
                static_cast<double>(access.sideInv),
                static_cast<double>(sets * eachSize),
                static_cast<double>(busSlots[i] + 1),
                static_cast<double>(toSlot + 1)
              };
              promises.emplace_back(action->mapTo(std::monostate{}));
              actions.emplace_back(std::move(action));
            }
          }
          factory.s.enqueueActionGroup(access.client, std::move(actions));
          return Promise<std::monostate>::all(promises);
        })->map(factory.alive, [this, slotsToFree](auto&&) {
          factory.busFree(*slotsToFree, false);
          slotsToFree->clear();
          return std::monostate{};
        })->finally(factory.alive, [this, slotsToFree]() {
          factory.busFree(*slotsToFree, true);
        }));
        break;
      }
      skip: {}
    }
    if (promises.empty())
      return scheduleTrivialPromise(factory.s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}

size_t ProcessCraftingRobot::mapCraftingGridSlot(size_t slot) {
  if (slot >= 7)
    slot += 2;
  else if (slot >= 4)
    slot += 1;
  return slot;
}

SharedPromise<std::monostate> ProcessCraftingRobot::cycle() {
  std::vector<SharedPromise<std::monostate>> promises;
  auto demands(factory.getDemand(recipes));
  for (auto &demand : demands) {
    auto &recipe(*demand.recipe);
    demand.in.clear();
    factory.resolveRecipeInputs(recipe, demand, true);
    demand.inAvail = std::min(demand.inAvail, recipe.data.first);
    if (demand.inAvail <= 0)
      continue;
    auto slotsToFreeIn(std::make_shared<std::vector<size_t>>());
    auto slotToFreeOut(std::make_shared<size_t>());
    std::vector<SharedPromise<size_t>> allocatedSlots;
    for (size_t i{}; i < recipe.in.size(); ++i) {
      allocatedSlots.emplace_back(factory.busAllocate()->then(factory.alive, [
        this, slotsToFreeIn, reservation(factory.reserve(name, demand.in[i], demand.inAvail * recipe.in[i].size))
      ](size_t busSlot) {
        slotsToFreeIn->emplace_back(busSlot);
        return reservation.extract(busSlot)->mapTo(busSlot);
      }));
    }
    allocatedSlots.emplace_back(factory.busAllocate()->map(factory.alive, [this, slotToFreeOut](size_t busSlot) {
      *slotToFreeOut = busSlot;
      return busSlot;
    }));
    promises.emplace_back(Promise<size_t>::all(allocatedSlots)->then(factory.alive, [
      this, sets(demand.inAvail), &recipe
    ](std::vector<size_t> &&busSlots) {
      auto &access(factory.s.getBestAccess(accesses));
      std::vector<SharedPromise<std::monostate>> promises;
      std::vector<SharedAction> actions;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        auto &in(recipe.in[i]);
        auto eachSize(in.size / static_cast<int>(in.data.size()));
        for (size_t toSlot : in.data) {
          /* select in */ {
            auto action(std::make_shared<Actions::Call>());
            action->inv = "robot";
            action->fn = "select";
            action->args = { static_cast<double>(mapCraftingGridSlot(toSlot)) };
            promises.emplace_back(action->mapTo(std::monostate{}));
            actions.emplace_back(std::move(action));
          }
          /* transfer in */ {
            auto action(std::make_shared<Actions::Call>());
            action->inv = "inventory_controller";
            action->fn = "suckFromSlot";
            action->args = {
              static_cast<double>(access.sideBus),
              static_cast<double>(busSlots[i] + 1),
              static_cast<double>(eachSize * sets)
            };
            promises.emplace_back(action->mapTo(std::monostate{}));
            actions.emplace_back(std::move(action));
          }
        }
      }
      for (auto &info : recipe.data.second) {
        /* select nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = "robot";
          action->fn = "select";
          action->args = { static_cast<double>(info.storageSlot) };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
        /* load nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = "robot";
          action->fn = "transferTo";
          action->args = { static_cast<double>(mapCraftingGridSlot(info.craftingGridSlot)) };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      /* select out */ {
        auto action(std::make_shared<Actions::Call>());
        action->inv = "robot";
        action->fn = "select";
        action->args = { 16.0 };
        promises.emplace_back(action->mapTo(std::monostate{}));
        actions.emplace_back(std::move(action));
      }
      /* craft */ {
        auto action(std::make_shared<Actions::Call>());
        action->inv = "crafting";
        action->fn = "craft";
        promises.emplace_back(action->mapTo(std::monostate{}));
        actions.emplace_back(std::move(action));
      }
      /* transfer out */ {
        auto action(std::make_shared<Actions::Call>());
        action->inv = "inventory_controller";
        action->fn = "dropIntoSlot";
        action->args = { static_cast<double>(access.sideBus), static_cast<double>(busSlots.back() + 1) };
        promises.emplace_back(action->mapTo(std::monostate{}));
        actions.emplace_back(std::move(action));
      }
      for (auto &info : recipe.data.second) {
        /* select nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = "robot";
          action->fn = "select";
          action->args = { static_cast<double>(mapCraftingGridSlot(info.craftingGridSlot)) };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
        /* store nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = "robot";
          action->fn = "transferTo";
          action->args = { static_cast<double>(info.storageSlot) };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      factory.s.enqueueActionGroup(access.client, std::move(actions));
      return Promise<std::monostate>::all(promises);
    })->map(factory.alive, [this, slotsToFreeIn](auto&&) {
      factory.busFree(*slotsToFreeIn, false);
      slotsToFreeIn->clear();
      return std::monostate{};
    })->finally(factory.alive, [this, slotsToFreeIn, slotToFreeOut]() {
      slotsToFreeIn->emplace_back(*slotToFreeOut);
      factory.busFree(*slotsToFreeIn, true);
    }));
  }
  if (promises.empty())
    return scheduleTrivialPromise(factory.s.io);
  return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
}

SharedPromise<std::monostate> ProcessRFToolsControlWorkbench::cycle() {
  std::vector<SharedPromise<std::monostate>> promises;
  auto demands(factory.getDemand(recipes));
  for (auto &demand : demands) {
    auto &recipe(*demand.recipe);
    demand.in.clear();
    factory.resolveRecipeInputs(recipe, demand, true);
    demand.inAvail = std::min(demand.inAvail, recipe.data.first);
    if (demand.inAvail <= 0)
      continue;
    auto slotsToFreeIn(std::make_shared<std::vector<size_t>>());
    auto slotToFreeOut(std::make_shared<size_t>());
    std::vector<SharedPromise<size_t>> allocatedSlots;
    for (size_t i{}; i < recipe.in.size(); ++i) {
      allocatedSlots.emplace_back(factory.busAllocate()->then(factory.alive, [
        this, slotsToFreeIn, reservation(factory.reserve(name, demand.in[i], demand.inAvail * recipe.in[i].size))
      ](size_t busSlot) {
        slotsToFreeIn->emplace_back(busSlot);
        return reservation.extract(busSlot)->mapTo(busSlot);
      }));
    }
    allocatedSlots.emplace_back(factory.busAllocate()->map(factory.alive, [this, slotToFreeOut](size_t busSlot) {
      *slotToFreeOut = busSlot;
      return busSlot;
    }));
    promises.emplace_back(Promise<size_t>::all(allocatedSlots)->then(factory.alive, [
      this, sets(demand.inAvail), &recipe
    ](std::vector<size_t> &&busSlots) {
      auto &access(factory.s.getBestAccess(accesses));
      std::vector<SharedPromise<std::monostate>> promises;
      std::vector<SharedAction> actions;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        auto &in(recipe.in[i]);
        auto eachSize(in.size / static_cast<int>(in.data.size()));
        for (size_t toSlot : in.data) {
          /* transfer in */ {
            auto action(std::make_shared<Actions::Call>());
            action->inv = access.addrIn;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(access.sideBusIn),
              static_cast<double>(Actions::down),
              static_cast<double>(eachSize * sets),
              static_cast<double>(busSlots[i] + 1),
              static_cast<double>(toSlot)
            };
            promises.emplace_back(action->mapTo(std::monostate{}));
            actions.emplace_back(std::move(action));
          }
        }
      }
      for (auto &info : recipe.data.second) {
        /* load nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = access.addrIn;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(access.sideNonConsumable),
            static_cast<double>(Actions::down),
            64.0,
            static_cast<double>(info.storageSlot),
            static_cast<double>(info.craftingGridSlot)
          };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      for (int i{}; i < sets; ++i) {
        /* transfer out */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = access.addrOut;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(Actions::up),
            static_cast<double>(access.sideBusOut),
            64.0,
            1.0,
            static_cast<double>(busSlots.back() + 1)
          };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      for (auto &info : recipe.data.second) {
        /* store nonConsumable */ {
          auto action(std::make_shared<Actions::Call>());
          action->inv = access.addrIn;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(Actions::down),
            static_cast<double>(access.sideNonConsumable),
            64.0,
            static_cast<double>(info.craftingGridSlot),
            static_cast<double>(info.storageSlot)
          };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      factory.s.enqueueActionGroup(access.client, std::move(actions));
      return Promise<std::monostate>::all(promises);
    })->map(factory.alive, [this, slotsToFreeIn](auto&&) {
      factory.busFree(*slotsToFreeIn, false);
      slotsToFreeIn->clear();
      return std::monostate{};
    })->finally(factory.alive, [this, slotToFreeOut, slotsToFreeIn]() {
      slotsToFreeIn->emplace_back(*slotToFreeOut);
      factory.busFree(*slotsToFreeIn, true);
    }));
  }
  if (promises.empty())
    return scheduleTrivialPromise(factory.s.io);
  return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
}

SharedPromise<std::monostate> ProcessBuffered::cycle() {
  if (!outFilter && stockList.empty())
    if (factory.getDemand(recipes).empty())
      return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<SharedItem, int, SharedItemHash, SharedItemEqual> inProcMap;
    auto quota(recipeMaxInProc);
    for (size_t slot{}; slot < items.size(); ++slot) {
      if (slotFilter && !slotFilter(slot)) {
        items[slot] = std::make_shared<ItemStack>(ItemStack{placeholderItem, 1});
        continue;
      }
      auto &stack(items[slot]);
      if (!stack)
        continue;
      inProcMap[stack->item] += stack->size;
      for (size_t i{}; i < stockList.size(); ++i)
        if (stockList[i].item->filter(*stack->item))
          goto skipSlot;
      quota -= stack->size;
      if (outFilter) {
        for (auto &recipe : recipes)
          for (auto &ingredient : recipe.in)
            if (ingredient.item->filter(*stack->item))
              goto skipSlot;
        if (outFilter(slot, *stack))
          promises.emplace_back(processOutput(slot, stack->item->maxSize));
      }
      skipSlot: {}
    }
    for (size_t i{}; i < stockList.size(); ++i) {
      auto &entry{stockList[i]};
      auto resolved{factory.getItem(*entry.item)};
      if (!resolved)
        continue;
      auto &inProc{inProcMap[resolved]};
      int toProc{std::min({entry.toStock - inProc, factory.getAvail(resolved, entry.allowBackup)})};
      if (toProc <= 0)
        continue;
      auto plan{insertIntoInventory(items, resolved, toProc)};
      if (plan.totalSize <= 0)
        continue;
      inProc += plan.totalSize;
      promises.emplace_back(factory.busAllocate()->then(factory.alive, [
        this, reservation(factory.reserve(name, resolved, plan.totalSize)), plan(std::move(plan.actions))
      ](size_t busSlot) {
        auto slotToFree(std::make_shared<std::optional<size_t>>(busSlot));
        return reservation.extract(busSlot)->then(factory.alive, [this, busSlot, plan(std::move(plan))](auto&&) {
          auto &access(factory.s.getBestAccess(accesses));
          std::vector<SharedPromise<std::monostate>> promises;
          std::vector<SharedAction> actions;
          for (auto &to : plan) {
            auto action(std::make_shared<Actions::Call>());
            action->inv = access.addr;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(access.sideBus),
              static_cast<double>(access.sideInv),
              static_cast<double>(to.second),
              static_cast<double>(busSlot + 1),
              static_cast<double>(to.first + 1)
            };
            promises.emplace_back(action->mapTo(std::monostate{}));
            actions.emplace_back(std::move(action));
          }
          factory.s.enqueueActionGroup(access.client, std::move(actions));
          return Promise<std::monostate>::all(promises);
        })->map(factory.alive, [this, slotToFree](auto&&) {
          factory.busFree(**slotToFree, false);
          slotToFree->reset();
          return std::monostate{};
        })->finally(factory.alive, [this, slotToFree]() {
          if (slotToFree->has_value()) {
            factory.busFree(**slotToFree, true);
          }
        });
      }));
    }
    if (quota > 0) {
      auto demands(factory.getDemand(recipes));
      for (auto &demand : demands) {
        auto &recipe(*demand.recipe);
        demand.in.clear();
        factory.resolveRecipeInputs(recipe, demand, true);
        if (demand.inAvail <= 0)
          continue;
        int listSum{};
        for (auto &ingredient : recipe.in)
          listSum += ingredient.size;
        int sets{std::min(demand.inAvail, quota / listSum)};
        if (sets <= 0)
          continue;
        std::vector<int*> inProcs;
        inProcs.reserve(recipe.in.size());
        int inProcSum{};
        for (auto &ingredient : demand.in)
          inProcSum += *inProcs.emplace_back(&inProcMap[ingredient]);
        sets = std::min(sets, (recipe.data - inProcSum) / listSum);
        if (sets <= 0)
          continue;
        auto backup{cloneInventory(items)};
        std::vector<InsertResult> plans;
        plans.reserve(recipe.in.size());
        retry: for (size_t i{}; i < recipe.in.size(); ++i) {
          int required{sets * recipe.in[i].size};
          auto plan{insertIntoInventory(items, demand.in[i], required)};
          if (plan.totalSize == required) {
            plans.emplace_back(std::move(plan));
          } else if (--sets > 0) {
            plans.clear();
            items = cloneInventory(backup);
            goto retry;
          } else {
            goto skipRecipe;
          }
        }
        {
          for (size_t i{}; i < recipe.in.size(); ++i)
            *inProcs[i] += plans[i].totalSize;
          quota -= sets * listSum;
          auto slotsToFree(std::make_shared<std::vector<size_t>>());
          std::vector<SharedPromise<size_t>> extractions;
          for (size_t i{}; i < recipe.in.size(); ++i) {
            extractions.emplace_back(factory.busAllocate()->then(factory.alive, [
              this, slotsToFree, reservation(factory.reserve(name, demand.in[i], plans[i].totalSize))
            ](size_t busSlot) {
              slotsToFree->emplace_back(busSlot);
              return reservation.extract(busSlot)->mapTo(busSlot);
            }));
          }
          promises.emplace_back(Promise<size_t>::all(extractions)->then(factory.alive, [
            this, plans(std::move(plans))
          ](std::vector<size_t> &&busSlots) {
            auto &access(factory.s.getBestAccess(accesses));
            std::vector<SharedPromise<std::monostate>> promises;
            std::vector<SharedAction> actions;
            for (size_t i{}; i < plans.size(); ++i) {
              for (auto &to : plans[i].actions) {
                auto action(std::make_shared<Actions::Call>());
                action->inv = access.addr;
                action->fn = "transferItem";
                action->args = {
                  static_cast<double>(access.sideBus),
                  static_cast<double>(access.sideInv),
                  static_cast<double>(to.second),
                  static_cast<double>(busSlots[i] + 1),
                  static_cast<double>(to.first + 1)
                };
                promises.emplace_back(action->mapTo(std::monostate{}));
                actions.emplace_back(std::move(action));
              }
            }
            factory.s.enqueueActionGroup(access.client, std::move(actions));
            return Promise<std::monostate>::all(promises);
          })->map(factory.alive, [this, slotsToFree](auto&&) {
            factory.busFree(*slotsToFree, false);
            slotsToFree->clear();
            return std::monostate{};
          })->finally(factory.alive, [this, slotsToFree]() {
            factory.busFree(*slotsToFree, true);
          }));
          if (quota <= 0)
            break;
        }
        skipRecipe: {}
      }
    }
    if (promises.empty())
      return scheduleTrivialPromise(factory.s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}

SharedPromise<std::monostate> ProcessScatteringWorkingSet::cycle() {
  if (!outFilter)
    if (factory.getDemand(recipes).empty())
      return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::vector<bool> isInSlot(items.size());
    for (size_t inSlot : inSlots) {
      if (inSlot >= items.size()) {
        items.resize(inSlot + 1);
        isInSlot.resize(inSlot + 1);
      }
      isInSlot[inSlot] = true;
    }
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (!isInSlot[slot] && stack && outFilter && outFilter(slot, *stack))
          promises.emplace_back(processOutput(slot, stack->size));
    }
    auto demands(factory.getDemand(recipes));
    for (auto &demand : demands) {
      auto &recipe(*demand.recipe);
      demand.in.clear();
      factory.resolveRecipeInputs(recipe, demand, true);
      int transferTotal{};
      std::unordered_map<size_t, int> transferMap;
      bool full{};
      while (demand.inAvail > 0) {
        int max{};
        std::optional<std::pair<int, size_t>> min;
        for (size_t slot : inSlots) {
          auto &stack(items[slot]);
          if (stack) {
            max = std::max(max, items[slot]->size);
            if (*stack->item == *demand.in.front())
              if (!min || stack->size < min->first)
                min.emplace(stack->size, slot);
            continue;
          }
          min.emplace(0, slot);
        }
        if (max >= eachSlotMaxInProc) {
          full = true;
          break;
        }
        if (!min.has_value() || min->first > max)
          break;
        --demand.inAvail;
        ++transferTotal;
        ++transferMap[min->second];
        auto &stack(items[min->second]);
        if (stack) {
          ++stack->size;
        } else {
          stack = std::make_shared<ItemStack>();
          stack->item = demand.in.front();
          stack->size = 1;
        }
      }
      if (transferTotal > 0) {
        promises.emplace_back(factory.busAllocate()->then(factory.alive, [
          this, transferMap(std::move(transferMap)), reservation(factory.reserve(name, demand.in.front(), transferTotal))
        ](size_t busSlot) {
          auto slotToFree(std::make_shared<std::optional<size_t>>(busSlot));
          return reservation.extract(busSlot)->then(factory.alive, [this, transferMap(std::move(transferMap)), busSlot](auto&&) {
            std::vector<SharedPromise<std::monostate>> promises;
            for (auto &transfer : transferMap) {
              auto &access(factory.s.getBestAccess(accesses));
              auto action(std::make_shared<Actions::Call>());
              action->inv = access.addr;
              action->fn = "transferItem";
              action->args = {
                static_cast<double>(access.sideBus),
                static_cast<double>(access.sideInv),
                static_cast<double>(transfer.second),
                static_cast<double>(busSlot + 1),
                static_cast<double>(transfer.first + 1)
              };
              promises.emplace_back(action->mapTo(std::monostate{}));
              factory.s.enqueueAction(access.client, std::move(action));
            }
            return Promise<std::monostate>::all(promises);
          })->map(factory.alive, [this, slotToFree](auto&&) {
            factory.busFree(**slotToFree, false);
            slotToFree->reset();
            return std::monostate{};
          })->finally(factory.alive, [this, slotToFree]() {
            if (slotToFree->has_value()) {
              factory.busFree(**slotToFree, true);
            }
          });
        }));
      }
      if (full)
        break;
    }
    if (promises.empty())
      return scheduleTrivialPromise(factory.s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}

namespace {
  struct InputlessInfo {
    int avail, needed;
  };
}

SharedPromise<std::monostate> ProcessInputless::cycle() {
  bool skip(true);
  for (auto &entry : entries) {
    if (factory.getAvail(factory.getItem(*entry.item), true) < entry.needed) {
      skip = false;
      break;
    }
  }
  if (skip)
    return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<SharedItem, InputlessInfo, SharedItemHash, SharedItemEqual> availMap;
    for (size_t slot{}; slot < items.size(); ++slot) {
      if (slotFilter && !slotFilter(slot))
        continue;
      auto &stack(items[slot]);
      if (!stack)
        continue;
      auto result(availMap.try_emplace(stack->item));
      auto &info(result.first->second);
      if (result.second) {
        info.avail = factory.getAvail(stack->item, true);
        info.needed = 0;
        for (auto &entry : entries)
          if (entry.item->filter(*stack->item))
            info.needed = std::max(info.needed, entry.needed);
      }
      int toProc(std::min(info.needed - info.avail, stack->size));
      if (toProc <= 0)
        continue;
      info.avail += toProc;
      promises.emplace_back(processOutput(slot, toProc));
    }
    if (promises.empty())
      return scheduleTrivialPromise(factory.s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}

SharedPromise<double> ProcessReactor::getPV() {
  if (factory.getAvail(factory.getItem(ItemFilters::Label("Cyanite Ingot")), true) < cyaniteNeeded) {
    auto result(std::make_shared<Promise<double>>());
    factory.s.io([result]() { result->onResult(0); });
    return result;
  } else if (hasTurbine) {
    std::vector<SharedPromise<SValue>> promises;
    {
      auto &access(factory.s.getBestAccess(accesses));
      auto action(std::make_shared<Actions::Call>());
      action->inv = access.addr;
      action->fn = "getHotFluidAmount";
      promises.emplace_back(action);
      factory.s.enqueueAction(access.client, std::move(action));
    }
    {
      auto &access(factory.s.getBestAccess(accesses));
      auto action(std::make_shared<Actions::Call>());
      action->inv = access.addr;
      action->fn = "getHotFluidAmountMax";
      promises.emplace_back(action);
      factory.s.enqueueAction(access.client, std::move(action));
    }
    return Promise<SValue>::all(promises)->then(factory.alive, [this](std::vector<SValue> &&args) {
      double pv;
      try {
        pv = std::get<double>(std::get<STable>(args[0]).at(1.0)) / std::get<double>(std::get<STable>(args[1]).at(1.0));
      } catch (std::exception &e) {
        return scheduleFailingPromise<double>(factory.s.io, name + ": " + e.what());
      }
      auto result(std::make_shared<Promise<double>>());
      factory.s.io([result, pv]() { result->onResult(pv); });
      return result;
    });
  } else {
    auto &access(factory.s.getBestAccess(accesses));
    auto action(std::make_shared<Actions::Call>());
    action->inv = access.addr;
    action->fn = "getEnergyStored";
    factory.s.enqueueAction(access.client, action);
    return action->then(factory.alive, [this](SValue &&args) {
      double pv;
      try {
        pv = std::get<double>(std::get<STable>(args).at(1.0)) / 10000000;
      } catch (std::exception &e) {
        return scheduleFailingPromise<double>(factory.s.io, name + ": " + e.what());
      }
      auto result(std::make_shared<Promise<double>>());
      factory.s.io([result, pv]() { result->onResult(pv); });
      return result;
    });
  }
}

SharedPromise<std::monostate> ProcessReactorHysteresis::cycle() {
  return getPV()->then(factory.alive, [this](double pv) {
    int on(-1);
    if (pv > upperBound && wasOn != 0)
      on = 0;
    else if (pv < lowerBound && wasOn != 1)
      on = 1;
    if (on < 0)
      return scheduleTrivialPromise(factory.s.io);
    factory.log(name + ": " + (on ? "on" : "off"), 0xff4fff);
    auto &access(factory.s.getBestAccess(accesses));
    auto action(std::make_shared<Actions::Call>());
    action->inv = access.addr;
    action->fn = "setActive";
    action->args = { static_cast<bool>(on) };
    factory.s.enqueueAction(access.client, action);
    return action->map(factory.alive, [this, on](auto&&) {
      wasOn = on;
      return std::monostate{};
    });
  });
}

SharedPromise<std::monostate> ProcessReactorProportional::cycle() {
  return getPV()->then(factory.alive, [this](double pv) {
    double rod{std::round(100 * pv)};
    int iRod(static_cast<int>(rod));
    factory.log(name + ": " + std::to_string(iRod) + "%", 0xff4fff);
    if (iRod == prev)
      return scheduleTrivialPromise(factory.s.io);
    auto &access(factory.s.getBestAccess(accesses));
    auto action(std::make_shared<Actions::Call>());
    action->inv = access.addr;
    action->fn = "setAllControlRodLevels";
    action->args = {rod};
    factory.s.enqueueAction(access.client, action);
    return action->map(factory.alive, [this, iRod](auto&&) {
      prev = iRod;
      return std::monostate{};
    });
  });
}

namespace {
  std::string toPercent(double x) {
    return std::to_string(static_cast<int>(std::round(x * 100))) + "%";
  }
}

SharedPromise<std::monostate> ProcessReactorPID::cycle() {
  return getPV()->then(factory.alive, [this](double pv) {
    double nowE((0.5 - pv) * 2);
    auto nowT(std::chrono::steady_clock::now());
    double diff(0);
    if (isInit) {
      isInit = false;
    } else {
      double ts(std::chrono::duration<double, std::chrono::seconds::period>(nowT - prevT).count());
      accum = std::clamp(accum + ts * nowE * kI, -1.0, 1.0);
      diff = (nowE - prevE) / ts;
    }
    prevT = nowT;
    prevE = nowE;
    double rawOut(nowE * kP + accum + diff * kD);
    int out(std::clamp(static_cast<int>(std::round(100 * (0.5 - rawOut))), 0, 100));
    factory.log(name + ": E=" + toPercent(-nowE) + ", I=" + toPercent(accum)
      + ", O=" + std::to_string(100 - out) + "%", 0xff4fff);
    if (out == prevOut)
      return scheduleTrivialPromise(factory.s.io);
    auto &access(factory.s.getBestAccess(accesses));
    auto action(std::make_shared<Actions::Call>());
    action->inv = access.addr;
    action->fn = "setAllControlRodLevels";
    action->args = {static_cast<double>(out)};
    factory.s.enqueueAction(access.client, action);
    return action->map(factory.alive, [this, out](auto&&) {
      prevOut = out;
      return std::monostate{};
    });
  });
}

const std::vector<std::string> ProcessPlasticMixer::colorMap{
  "Black", "Red", "Green", "Brown", "Blue", "Purple", "Cyan", "Light Gray",
  "Gray", "Pink", "Lime", "Yellow", "Light Blue", "Magenta", "Orange", "White"
};

SharedPromise<std::monostate> ProcessPlasticMixer::cycle() {
  std::vector<int> avails;
  avails.reserve(colorMap.size());
  for (auto &i : colorMap)
    avails.emplace_back(factory.getAvail(factory.getItem(ItemFilters::Label{i + " Plastic"}), true));
  int which{static_cast<int>(std::min_element(avails.begin(), avails.end()) - avails.begin())};
  if (avails[which] >= needed) {
    factory.log(name + ": off", 0xff4fff);
    which = 0;
  } else {
    factory.log(name + ": making " + colorMap[which] + " Plastic", 0xff4fff);
    ++which;
  }
  if (which == prev)
    return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "selectColor";
  action->args = {static_cast<double>(which)};
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [this, which](auto&&) {
    prev = which;
    return std::monostate{};
  });
}

SharedPromise<std::monostate> ProcessRedstoneConditional::cycle() {
  if (precondition && !precondition())
    return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "getInput";
  action->args = {static_cast<double>(access.side)};
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](SValue &&arg) {
    double level;
    try {
      level = std::get<double>(std::get<STable>(arg).at(1.0));
    } catch (std::exception &e) {
      return scheduleFailingPromise<std::monostate>(factory.s.io, name + ": " + e.what());
    }
    if (predicate(static_cast<int>(level))) {
      return child->cycle();
    } else {
      if (logSkip)
        factory.log(name + ": skipped", 0xff0000);
      return scheduleTrivialPromise(factory.s.io);
    }
  });
}

SharedPromise<std::monostate> ProcessRedstoneEmitter::cycle() {
  int value(valueFn());
  if (value == prevValue)
    return scheduleTrivialPromise(factory.s.io);
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "setOutput";
  action->args = {static_cast<double>(access.side), static_cast<double>(value)};
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [this, value](auto&&) {
    prevValue = value;
    return std::monostate{};
  });
}

std::function<int()> ProcessRedstoneEmitter::makeNeeded(Factory &factory, std::string name, SharedItemFilter item, int toStock) {
  return [&factory, name(std::move(name)), item(std::move(item)), toStock]() {
    if (factory.getAvail(factory.getItem(*item), true) < toStock) {
      factory.log(name + ": on", 0xff4fff);
      return 15;
    } else {
      return 0;
    }
  };
}

SharedPromise<std::monostate> ProcessFluxNetwork::cycle() {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "getEnergyInfo";
  factory.s.enqueueAction(access.client, action);
  return action->then(factory.alive, [this](SValue &&arg) {
    try {
      lastEnergy = std::get<double>(std::get<STable>(std::get<STable>(arg).at(1.0)).at("totalEnergy"));
    } catch (std::exception &e) {
      return scheduleFailingPromise<std::monostate>(factory.s.io, name + ": " + e.what());
    }
    std::ostringstream os;
    os << name << ": " << std::setprecision(0) << std::fixed << lastEnergy;
    factory.log(os.str(), 0xff4fff);
    if (outputs.empty())
      return scheduleTrivialPromise(factory.s.io);
    std::vector<SharedPromise<std::monostate>> promises;
    for (auto &output : outputs)
      promises.emplace_back(output->cycle());
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}
