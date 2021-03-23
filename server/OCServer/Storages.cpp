#include "Storages.h"

SharedPromise<std::monostate> StorageDrawer::update() {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    for (size_t slot{}; slot < items.size(); ++slot) {
      auto &stack(items[slot]);
      if (stack)
        factory.getOrAddItemInfo(stack->item).addProvider(
          std::make_unique<ProviderDrawer>(*this, slot, stack->size));
    }
    return std::monostate{};
  });
}

std::optional<int> StorageDrawer::sinkPriority(const Item &item) {
  for (auto &i : filters)
    if (i->filter(item))
      return std::numeric_limits<int>::max();
  return std::nullopt;
}

std::pair<int, SharedPromise<std::monostate>> StorageDrawer::sink(const ItemStack &stack, size_t slot) {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideBus),
    static_cast<double>(access.sideInv),
    static_cast<double>(stack.size),
    static_cast<double>(slot + 1)
  };
  factory.s.enqueueAction(access.client, action);
  return {stack.size, action->mapTo(std::monostate{})};
}

SharedPromise<std::monostate> ProviderDrawer::extract(int size, size_t slot) {
  auto &access(factory.s.getBestAccess(drawer.accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideInv),
    static_cast<double>(access.sideBus),
    static_cast<double>(size),
    static_cast<double>(this->slot + 1),
    static_cast<double>(slot + 1)
  };
  factory.s.enqueueAction(access.client, action);
  return action->mapTo(std::monostate{});
}

SharedPromise<std::monostate> StorageChest::update() {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::List>());
  action->inv = access.addr;
  action->side = access.sideInv;
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    content = std::move(items);
    for (size_t slot{}; slot < content.size(); ++slot) {
      auto &stack(content[slot]);
      if (stack)
        factory.getOrAddItemInfo(stack->item).addProvider(
          std::make_unique<ProviderChest>(*this, slot, stack->size));
    }
    return std::monostate{};
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
      emptySlot.emplace(slot);
    }
  }
  if (maxSize.has_value()) {
    return maxSize;
  } else if (emptySlot.has_value()) {
    slotToSink = *emptySlot;
    return std::numeric_limits<int>::min() + 1;
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
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideBus),
    static_cast<double>(access.sideInv),
    static_cast<double>(toProc),
    static_cast<double>(slot + 1),
    static_cast<double>(slotToSink + 1)
  };
  factory.s.enqueueAction(access.client, action);
  return {toProc, action->mapTo(std::monostate{})};
}

SharedPromise<std::monostate> ProviderChest::extract(int size, size_t slot) {
  auto &access(factory.s.getBestAccess(chest.accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideInv),
    static_cast<double>(access.sideBus),
    static_cast<double>(size),
    static_cast<double>(this->slot + 1),
    static_cast<double>(slot + 1)
  };
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [&chest(chest), size, slot(this->slot)](auto&&) {
    auto &stack(chest.content[slot]);
    stack->size -= size;
    if (stack->size <= 0)
      stack.reset();
    return std::monostate{};
  });
}

SharedPromise<std::monostate> StorageME::update() {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::ListME>());
  action->inv = access.me;
  factory.s.enqueueAction(access.client, action);
  return action->map(factory.alive, [this](std::vector<SharedItemStack> &&items) {
    for (auto &stack : items) {
      stack->item->others.erase("isCraftable");
      factory.getOrAddItemInfo(stack->item).addProvider(
        std::make_unique<ProviderME>(*this, stack->item, stack->size));
    }
    return std::monostate{};
  });
}

std::pair<int, SharedPromise<std::monostate>> StorageME::sink(const ItemStack &stack, size_t slot) {
  auto &access(factory.s.getBestAccess(accesses));
  auto action(std::make_shared<Actions::Call>());
  action->inv = access.addr;
  action->fn = "transferItem";
  action->args = {
    static_cast<double>(access.sideBus),
    static_cast<double>(access.sideInv),
    static_cast<double>(stack.size),
    static_cast<double>(slot + 1),
    9.0
  };
  factory.s.enqueueAction(access.client, action);
  return {stack.size, action->mapTo(std::monostate{})};
}

SharedPromise<std::monostate> ProviderME::extract(int size, size_t slot) {
  auto itr(me.accessForItem.try_emplace(item));
  if (itr.second)
    itr.first->second = &factory.s.getBestAccess(me.accesses);
  auto &access(*itr.first->second);
  auto action(std::make_shared<Actions::XferME>());
  item->serialize(action->filter);
  action->size = size;
  action->inv = access.addr;
  action->me = access.me;
  action->entry = access.entry;
  action->args = {
    static_cast<double>(access.sideInv),
    static_cast<double>(access.sideBus),
    static_cast<double>(size),
    static_cast<double>(access.entry),
    static_cast<double>(slot + 1)
  };
  factory.s.enqueueAction(access.client, action);
  return action->mapTo(std::monostate{});
}
