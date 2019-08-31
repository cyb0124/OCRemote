#include <set>
#include "Processes.h"

SharedPromise<std::monostate> ProcessSingleBlock::processOutput(size_t slot, int size) {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(sideCrafter),
    static_cast<double>(sideBus),
    static_cast<double>(size),
    static_cast<double>(slot + 1)
  };
  return factory.busAllocate()->then(factory.alive, [this, action(std::move(action))](size_t busSlot) {
    action->args.push_back(static_cast<double>(busSlot + 1));
    factory.s.enqueueAction(client, action);
    return action->mapTo(std::monostate{})->finally(factory.alive, [busSlot, this]() { factory.busFree(busSlot);});
  });
}

SharedPromise<std::monostate> ProcessSlotted::cycle() {
  if (!outFilter)
    if (factory.getDemand(recipes).empty())
      return scheduleTrivialPromise(factory.s.io);
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
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
            sets = std::min(sets, (recipe.data - inProc) / eachSize);
            if (sets <= 0)
              goto skip;
          }
        }
      }
      for (auto &info : slotInfos) {
        if (info.second && usedSlots.find(info.first) == usedSlots.end())
          goto skip;
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
          std::vector<SharedPromise<std::monostate>> promises;
          std::vector<SharedAction> actions;
          for (size_t i{}; i < recipe.in.size(); ++i) {
            auto &in(recipe.in[i]);
            auto eachSize(in.size / static_cast<int>(in.data.size()));
            for (size_t toSlot : in.data) {
              auto action(std::make_shared<Actions::Call>());
              action->inv = inv;
              action->fn = "transferItem";
              action->args = {
                static_cast<double>(sideBus),
                static_cast<double>(sideCrafter),
                static_cast<double>(sets * eachSize),
                static_cast<double>(busSlots[i] + 1),
                static_cast<double>(toSlot + 1)
              };
              promises.emplace_back(action->mapTo(std::monostate{}));
              actions.emplace_back(std::move(action));
            }
          }
          factory.s.enqueueActionGroup(client, std::move(actions));
          return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
        })->finally(factory.alive, [this, slotsToFree(std::move(slotsToFree))]() {
            factory.busFree(*slotsToFree);
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
    auto slotsToFree(std::make_shared<std::vector<size_t>>());
    std::vector<SharedPromise<size_t>> allocatedSlots;
    for (size_t i{}; i < recipe.in.size(); ++i) {
      allocatedSlots.emplace_back(factory.busAllocate()->then(factory.alive, [
        this, slotsToFree, reservation(factory.reserve(name, demand.in[i], demand.inAvail * recipe.in[i].size))
      ](size_t busSlot) {
        slotsToFree->emplace_back(busSlot);
        return reservation.extract(busSlot)->mapTo(busSlot);
      }));
    }
    allocatedSlots.emplace_back(factory.busAllocate()->map(factory.alive, [this, slotsToFree](size_t busSlot) {
      slotsToFree->emplace_back(busSlot);
      return busSlot;
    }));
    promises.emplace_back(Promise<size_t>::all(allocatedSlots)->then(factory.alive, [this, sets(demand.inAvail), &recipe](std::vector<size_t> &&busSlots) {
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
              static_cast<double>(sideBus),
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
        action->args = { static_cast<double>(sideBus), static_cast<double>(busSlots.back() + 1) };
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
      factory.s.enqueueActionGroup(client, std::move(actions));
      return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
    })->finally(factory.alive, [this, slotsToFree(std::move(slotsToFree))]() {
      factory.busFree(*slotsToFree);
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
    auto slotsToFree(std::make_shared<std::vector<size_t>>());
    std::vector<SharedPromise<size_t>> allocatedSlots;
    for (size_t i{}; i < recipe.in.size(); ++i) {
      allocatedSlots.emplace_back(factory.busAllocate()->then(factory.alive, [
        this, slotsToFree, reservation(factory.reserve(name, demand.in[i], demand.inAvail * recipe.in[i].size))
      ](size_t busSlot) {
        slotsToFree->emplace_back(busSlot);
        return reservation.extract(busSlot)->mapTo(busSlot);
      }));
    }
    allocatedSlots.emplace_back(factory.busAllocate()->map(factory.alive, [this, slotsToFree](size_t busSlot) {
      slotsToFree->emplace_back(busSlot);
      return busSlot;
    }));
    promises.emplace_back(Promise<size_t>::all(allocatedSlots)->then(factory.alive, [this, sets(demand.inAvail), &recipe](std::vector<size_t> &&busSlots) {
      std::vector<SharedPromise<std::monostate>> promises;
      std::vector<SharedAction> actions;
      for (size_t i{}; i < recipe.in.size(); ++i) {
        auto &in(recipe.in[i]);
        auto eachSize(in.size / static_cast<int>(in.data.size()));
        for (size_t toSlot : in.data) {
          /* transfer in */ {
            auto action(std::make_shared<Actions::Call>());
            action->inv = invIn;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(sideBusIn),
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
          action->inv = invIn;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(sideNonConsumable),
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
          action->inv = invOut;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(Actions::up),
            static_cast<double>(sideBusOut),
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
          action->inv = invIn;
          action->fn = "transferItem";
          action->args = {
            static_cast<double>(Actions::down),
            static_cast<double>(sideNonConsumable),
            64.0,
            static_cast<double>(info.craftingGridSlot),
            static_cast<double>(info.storageSlot)
          };
          promises.emplace_back(action->mapTo(std::monostate{}));
          actions.emplace_back(std::move(action));
        }
      }
      factory.s.enqueueActionGroup(client, std::move(actions));
      return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
    })->finally(factory.alive, [this, slotsToFree(std::move(slotsToFree))]() {
      factory.busFree(*slotsToFree);
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
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<SharedItem, int, SharedItemHash, SharedItemEqual> inProcMap;
    auto quota{recipeMaxInProc};
    for (size_t slot{}; slot < items.size(); ++slot) {
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
        return reservation.extract(busSlot)->then(factory.alive, [this, busSlot, plan(std::move(plan))](auto&&) {
          std::vector<SharedPromise<std::monostate>> promises;
          std::vector<SharedAction> actions;
          for (auto &to : plan) {
            auto action(std::make_shared<Actions::Call>());
            action->inv = inv;
            action->fn = "transferItem";
            action->args = {
              static_cast<double>(sideBus),
              static_cast<double>(sideCrafter),
              static_cast<double>(to.second),
              static_cast<double>(busSlot + 1),
              static_cast<double>(to.first + 1)
            };
            promises.emplace_back(action->mapTo(std::monostate{}));
            actions.emplace_back(std::move(action));
          }
          factory.s.enqueueActionGroup(client, std::move(actions));
          return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
        })->finally(factory.alive, [this, busSlot]() {
          factory.busFree(busSlot);
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
            std::vector<SharedPromise<std::monostate>> promises;
            std::vector<SharedAction> actions;
            for (size_t i{}; i < plans.size(); ++i) {
              for (auto &to : plans[i].actions) {
                auto action(std::make_shared<Actions::Call>());
                action->inv = inv;
                action->fn = "transferItem";
                action->args = {
                  static_cast<double>(sideBus),
                  static_cast<double>(sideCrafter),
                  static_cast<double>(to.second),
                  static_cast<double>(busSlots[i] + 1),
                  static_cast<double>(to.first + 1)
                };
                promises.emplace_back(action->mapTo(std::monostate{}));
                actions.emplace_back(std::move(action));
              }
            }
            factory.s.enqueueActionGroup(client, std::move(actions));
            return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
          })->finally(factory.alive, [this, slotsToFree(std::move(slotsToFree))]() {
            factory.busFree(*slotsToFree);
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
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
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
          return reservation.extract(busSlot)->then(factory.alive, [this, transferMap(std::move(transferMap)), busSlot](auto&&) {
            std::vector<SharedPromise<std::monostate>> promises;
            std::vector<SharedAction> actions;
            for (auto &transfer : transferMap) {
              auto action(std::make_shared<Actions::Call>());
              action->inv = inv;
              action->fn = "transferItem";
              action->args = {
                static_cast<double>(sideBus),
                static_cast<double>(sideCrafter),
                static_cast<double>(transfer.second),
                static_cast<double>(busSlot + 1),
                static_cast<double>(transfer.first + 1)
              };
              promises.emplace_back(action->mapTo(std::monostate{}));
              actions.emplace_back(std::move(action));
            }
            factory.s.enqueueActionGroup(client, std::move(actions));
            return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
          })->finally(factory.alive, [this, busSlot]() {
            factory.busFree(busSlot);
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

SharedPromise<std::monostate> ProcessInputless::cycle() {
  auto needed(this->needed());
  if (needed <= 0)
    return scheduleTrivialPromise(factory.s.io);
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then(factory.alive, [this, needed](std::vector<SharedItemStack> &&items) {
    if (sourceSlot >= items.size() || !items[sourceSlot])
      skip: return scheduleTrivialPromise(factory.s.io);
    int toProc{std::min(needed, items[sourceSlot]->size)};
    if (toProc <= 0)
      goto skip;
    return processOutput(sourceSlot, toProc);
  });
}

std::function<int()> ProcessInputless::makeNeeded(Factory &factory, SharedItemFilter item, int toStock) {
  return [&factory, item(std::move(item)), toStock]() {
    return toStock - factory.getAvail(factory.getItem(*item), true);
  };
}

SharedPromise<std::monostate> ProcessHeterogeneousInputless::cycle() {
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideCrafter;
  factory.s.enqueueAction(client, action);
  return action->then(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    std::vector<SharedPromise<std::monostate>> promises;
    std::unordered_map<SharedItem, int, SharedItemHash, SharedItemEqual> availMap;
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (!stack)
        continue;
      int &avail(availMap.try_emplace(stack->item, factory.getAvail(stack->item, true)).first->second);
      int toProc{std::min(needed - avail, stack->size)};
      if (toProc <= 0)
        continue;
      avail += toProc;
      promises.emplace_back(processOutput(slot, toProc));
    }
    if (promises.empty())
      return scheduleTrivialPromise(factory.s.io);
    return Promise<std::monostate>::all(promises)->mapTo(std::monostate{});
  });
}

SharedPromise<std::monostate> ProcessReactorHysteresis::cycle() {
  std::vector<SharedPromise<SValue>> promises;
  std::vector<SharedAction> actions;
  {
    auto action(std::make_shared<Actions::Call>());
    action->inv = inv;
    action->fn = "getActive";
    promises.emplace_back(action);
    actions.emplace_back(std::move(action));
  }
  {
    auto action(std::make_shared<Actions::Call>());
    action->inv = inv;
    action->fn = "getEnergyStored";
    promises.emplace_back(action);
    actions.emplace_back(std::move(action));
  }
  factory.s.enqueueActionGroup(client, actions);
  return Promise<SValue>::all(promises)->then(factory.alive, [this](std::vector<SValue> &&args) {
    bool wasOn;
    int level;
    try {
      wasOn = std::get<bool>(std::get<STable>(args.front()).at(1.0));
      level = static_cast<int>(std::get<double>(std::get<STable>(args.back()).at(1.0)));
    } catch (std::exception &e) {
      return scheduleFailingPromise<std::monostate>(factory.s.io, name + ": " + e.what());
    }
    std::optional<bool> on;
    if (wasOn) {
      if (level > upperBound)
        on.emplace(false);
    } else {
      if (level < lowerBound)
        on.emplace(true);
    }
    if (!on.has_value())
      return scheduleTrivialPromise(factory.s.io);
    factory.log(name + ": " + (*on ? "on" : "off"), 0xff4fff);
    auto action(std::make_shared<Actions::Call>());
    action->inv = inv;
    action->fn = "setActive";
    action->args = { *on };
    factory.s.enqueueAction(client, action);
    return action->mapTo(std::monostate{});
  });
}

SharedPromise<std::monostate> ProcessReactorProportional::cycle() {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "getEnergyStored";
  factory.s.enqueueAction(client, action);
  return action->then(factory.alive, [this](SValue &&arg) {
    double level;
    try {
      level = std::get<double>(std::get<STable>(arg).at(1.0));
    } catch (std::exception &e) {
      return scheduleFailingPromise<std::monostate>(factory.s.io, name + ": " + e.what());
    }
    double rod{std::round(100 * level / 10000000)};
    factory.log(name + ": " + std::to_string(static_cast<int>(rod)) + "%", 0xff4fff);
    auto action(std::make_shared<Actions::Call>());
    action->inv = inv;
    action->fn = "setAllControlRodLevels";
    action->args = {rod};
    factory.s.enqueueAction(client, action);
    return action->mapTo(std::monostate{});
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
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "selectColor";
  action->args = {static_cast<double>(which)};
  factory.s.enqueueAction(client, action);
  return action->mapTo(std::monostate{});
}

SharedPromise<std::monostate> ProcessRedstoneConditional::cycle() {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "getInput";
  action->args = {static_cast<double>(side)};
  factory.s.enqueueAction(client, action);
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
