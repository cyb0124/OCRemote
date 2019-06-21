#include "Storages.h"

SharedPromise<std::monostate> StorageDrawer::update() {
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideDrawer;
  factory.s.enqueueAction(client, action);
  return action->map([this, wk(std::weak_ptr(factory.alive))](std::vector<SharedItemStack> &&items) -> std::monostate {
    if (wk.expired()) return {};
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (stack)
        factory.getOrAddItemInfo(stack->item).addProvider(
          std::make_unique<ProviderDrawer>(*this, slot, stack->size));
    }
  });
}

std::optional<int> StorageDrawer::sinkPriority(const Item &item) {
  for (auto &i : filters)
    if (i->filter(item))
      return std::numeric_limits<int>::max();
  return std::nullopt;
}

std::pair<int, SharedPromise<std::monostate>> StorageDrawer::sink(const ItemStack &stack, size_t slot) {
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(sideBus),
    static_cast<double>(sideDrawer),
    static_cast<double>(stack.size),
    static_cast<double>(slot)
  };
  factory.s.enqueueAction(client, action);
  return {stack.size, action->map([](auto&&) { return std::monostate{}; })};
}

SharedPromise<std::monostate> ProviderDrawer::extract(int size, int slot) {
  Provider::extract(size, slot);
  auto action(std::make_shared<Actions::Call>());
  action->inv = drawer.inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(drawer.sideDrawer),
    static_cast<double>(drawer.sideBus),
    static_cast<double>(size),
    static_cast<double>(this->slot),
    static_cast<double>(slot)
  };
  factory.s.enqueueAction(drawer.client, action);
  return action->map([](auto&&) { return std::monostate{}; });
}

SharedPromise<std::monostate> StorageChest::update() {
  auto action(std::make_shared<Actions::List>());
  action->inv = inv;
  action->side = sideChest;
  factory.s.enqueueAction(client, action);
  return action->map([this, wk(std::weak_ptr(factory.alive))](std::vector<SharedItemStack> &&items) -> std::monostate {
    if (wk.expired()) return {};
    content = std::move(items);
    for (size_t slot{}; slot < content.size(); ++slot) {
      auto &stack(content[slot]);
      if (stack)
        factory.getOrAddItemInfo(stack->item).addProvider(
          std::make_unique<ProviderChest>(*this, slot, stack->size));
    }
  });
}

std::optional<int> StorageChest::sinkPriority(const Item &item) {
  std::optional<size_t> emptySlot;
  std::optional<int> maxSize;
  for (size_t slot{}; slot < content.size(); ++slot) {
    auto &stack(content[slot]);
    if (stack) {
      if (*stack->item == item && stack->size < item.maxSize) {
        if (!maxSize.has_value() || stack->size > *maxSize) {
          maxSize = stack->size;
          slotToSink = slot;
        }
      }
    } else {
      emptySlot = slot;
    }
  }
  if (maxSize.has_value()) {
    return maxSize;
  } else if (emptySlot.has_value()) {
    slotToSink = *emptySlot;
    return std::numeric_limits<int>::min();
  } else {
    return std::nullopt;
  }
}

std::pair<int, SharedPromise<std::monostate>> StorageChest::sink(const ItemStack &stack, size_t slot) {
  int toProc;
  auto &dstStack(content[slotToSink]);
  if (dstStack) {
    toProc = std::min(stack.size, dstStack->item->maxSize - dstStack->size);
    dstStack->size += toProc;
  } else {
    toProc = stack.size;
    dstStack = std::make_shared<ItemStack>(stack);
  }
  auto action(std::make_shared<Actions::Call>());
  action->inv = inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(sideBus),
    static_cast<double>(sideChest),
    static_cast<double>(toProc),
    static_cast<double>(slot),
    static_cast<double>(slotToSink)
  };
  factory.s.enqueueAction(client, action);
  return {toProc, action->map([](auto&&) { return std::monostate{}; })};
}

SharedPromise<std::monostate> ProviderChest::extract(int size, int slot) {
  Provider::extract(size, slot);
  auto action(std::make_shared<Actions::Call>());
  action->inv = chest.inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(chest.sideChest),
    static_cast<double>(chest.sideBus),
    static_cast<double>(size),
    static_cast<double>(this->slot),
    static_cast<double>(slot)
  };
  factory.s.enqueueAction(chest.client, action);
  return action->map([wk(std::weak_ptr(factory.alive)), &chest(chest), size, slot](auto&&) -> std::monostate {
    if (wk.expired())
      return {};
    auto &stack(chest.content[slot]);
    stack->size -= size;
    if (stack->size <= 0)
      stack.reset();
    return {};
  });
}

AccessME &StorageME::getBestAccess() {
  AccessME *bestAccess{&accesses.front()};
  size_t bestCount{std::numeric_limits<size_t>::max()};
  for (auto &access : accesses) {
    size_t count{factory.s.countPending(access.client)};
    if (count < bestCount) {
      bestCount = count;
      bestAccess = &access;
    }
  }
  return *bestAccess;
}

SharedPromise<std::monostate> StorageME::update() {
  auto &access(getBestAccess());
  auto action(std::make_shared<Actions::ListME>());
  action->inv = access.me;
  factory.s.enqueueAction(access.client, action);
  return action->map([this, wk(std::weak_ptr(factory.alive))](std::vector<SharedItemStack> &&items) -> std::monostate {
    if (wk.expired()) return {};
    for (auto &stack : items)
      factory.getOrAddItemInfo(stack->item).addProvider(
        std::make_unique<ProviderME>(*this, stack->item, stack->size));
    return {};
  });
}

std::pair<int, SharedPromise<std::monostate>> StorageME::sink(const ItemStack &stack, size_t slot) {
  auto &access(getBestAccess());
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.inv;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideBus),
    static_cast<double>(access.sideME),
    static_cast<double>(stack.size),
    static_cast<double>(slot),
    9.0
  };
  factory.s.enqueueAction(access.client, action);
  return {stack.size, action->map([](auto&&) { return std::monostate{}; })};
}

SharedPromise<std::monostate> ProviderME::extract(int size, int slot) {
  Provider::extract(size, slot);
  auto &access(me.getBestAccess());
  auto action(std::make_shared<Actions::XferME>());
  item->serialize(action->filter);
  action->size = size;
  action->inv = access.inv;
  action->me = access.me;
  action->args = {
    static_cast<double>(access.sideME),
    static_cast<double>(access.sideBus),
    static_cast<double>(size),
    1.0,
    static_cast<double>(slot)
  };
  factory.s.enqueueAction(access.client, action);
  return action->map([](auto&&) { return std::monostate{}; });
}
