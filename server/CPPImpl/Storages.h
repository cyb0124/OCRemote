#ifndef _STORAGES_H_
#define _STORAGES_H_
#include "Factory.h"

struct StorageDrawer : public Storage {
  std::vector<AccessInv> accesses;
  std::vector<SharedItemFilter> filters;
  StorageDrawer(Factory &factory, decltype(accesses) accesses, decltype(filters) filters)
    :Storage(factory), accesses(std::move(accesses)), filters(std::move(filters)) {}
  SharedPromise<std::monostate> update() override;
  std::optional<int> sinkPriority(const Item&) override;
  std::pair<int, SharedPromise<std::monostate>> sink(const ItemStack&, size_t slot) override;
};

struct ProviderDrawer : Provider {
  StorageDrawer &drawer;
  size_t slot;
  ProviderDrawer(StorageDrawer &drawer, size_t slot, int size)
    :Provider(drawer.factory, size, std::numeric_limits<int>::min()), drawer(drawer), slot(slot) {}
  SharedPromise<std::monostate> extract(int size, size_t slot) override;
};

struct StorageChest : Storage {
  std::vector<AccessInv> accesses;
  std::vector<SharedItemStack> content;
  size_t slotToSink;
  StorageChest(Factory &factory, decltype(accesses) accesses)
    :Storage(factory), accesses(std::move(accesses)) {}
  void endOfCycle() override { content.clear(); }
  SharedPromise<std::monostate> update() override;
  std::optional<int> sinkPriority(const Item&) override;
  std::pair<int, SharedPromise<std::monostate>> sink(const ItemStack&, size_t slot) override;
};

struct ProviderChest : Provider {
  StorageChest &chest;
  size_t slot;
  ProviderChest(StorageChest &chest, size_t slot, int size)
    :Provider(chest.factory, size, -size), chest(chest), slot(slot) {}
  SharedPromise<std::monostate> extract(int size, size_t slot) override;
};

struct AccessME : AccessInv {
  std::string me;
  int entry;
  AccessME(std::string client, std::string addr, int sideInv, int sideBus, int entry, std::string me = "me_interface")
    :AccessInv(std::move(client), std::move(addr), sideInv, sideBus), me(std::move(me)), entry(entry) {}
};

struct StorageME : Storage {
  std::vector<AccessME> accesses;
  std::unordered_map<std::shared_ptr<Item>, const AccessME*, SharedItemHash, SharedItemEqual> accessForItem;
  StorageME(Factory &factory, std::vector<AccessME> accesses) :Storage(factory), accesses(std::move(accesses)) {}
  void endOfCycle() override { accessForItem.clear(); }
  SharedPromise<std::monostate> update() override;
  std::optional<int> sinkPriority(const Item&) override { return std::numeric_limits<int>::max(); }
  std::pair<int, SharedPromise<std::monostate>> sink(const ItemStack&, size_t slot) override;
};

struct ProviderME : Provider {
  StorageME &me;
  SharedItem item;
  ProviderME(StorageME &me, SharedItem item, int size)
    :Provider(me.factory, size, -std::numeric_limits<int>::max()), me(me), item(std::move(item)) {}
  SharedPromise<std::monostate> extract(int size, size_t slot) override;
};

#endif
