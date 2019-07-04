#pragma once
#include "Factory.h"

struct StorageDrawer : public Storage {
  std::string client, inv;
  int sideDrawer, sideBus;
  std::vector<SharedItemFilter> filters;
  StorageDrawer(Factory &factory, std::string client, std::string inv,
    int sideDrawer, int sideBus, std::vector<SharedItemFilter> filters)
    :Storage(factory), client(std::move(client)), inv(std::move(inv)),
    sideDrawer(sideDrawer), sideBus(sideBus), filters(std::move(filters)) {}
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
  std::string client, inv;
  int sideChest, sideBus;
  std::vector<SharedItemStack> content;
  size_t slotToSink;
  StorageChest(Factory &factory, std::string client, std::string inv, int sideChest, int sideBus)
    :Storage(factory), client(std::move(client)), inv(std::move(inv)), sideChest(sideChest), sideBus(sideBus) {}
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

struct AccessME {
  std::string client, inv, me;
  int sideME, sideBus, entry;
  AccessME(std::string client, std::string inv, int sideME, int sideBus, int entry, std::string me = "me_interface")
    :client(std::move(client)), inv(std::move(inv)), me(std::move(me)), sideME(sideME), sideBus(sideBus), entry(entry) {}
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
