#pragma once
#include "Factory.h"

struct ProcessSingleBlock : Process {
  std::string client, inv;
  int sideCrafter, sideBus;
  ProcessSingleBlock(Factory &factory, std::string client, std::string inv, int sideCrafter, int sideBus)
    :Process(factory), client(std::move(client)), inv(std::move(inv)), sideCrafter(sideCrafter), sideBus(sideBus) {}
  SharedPromise<std::monostate> processOutput(size_t slot, int size);
};

struct ProcessSlotted : ProcessSingleBlock {
  using Recipe = ::Recipe<int, std::vector<size_t>>; // eachSlotMaxInProc, slots
  std::string name;
  std::vector<size_t> inSlots;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  ProcessSlotted(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, decltype(inSlots) inSlots,
    decltype(outFilter) outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(client), std::move(inv), sideCrafter, sideBus),
    name(std::move(name)), inSlots(std::move(inSlots)), outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessCraftingRobot : Process {
  // Note: can't craft more than one stack at a time.
  // Slots: 1, 2, 3
  //        4, 5, 6
  //        7, 8, 9
  using Recipe = ::Recipe<int, std::vector<size_t>>; // maxSets, slots
  std::string name, client;
  int sideBus;
  std::vector<Recipe> recipes;
  ProcessCraftingRobot(Factory &factory, std::string name, std::string client,
    int sideBus, std::vector<Recipe> recipes) :Process(factory), name(std::move(name)),
    client(std::move(client)), sideBus(sideBus), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessWorkingSet : ProcessSingleBlock {
  // Note: single input only.
  using Recipe = ::Recipe<std::pair<std::string, int>>; // name, maxInproc
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  ProcessWorkingSet(Factory &factory, std::string client,
    std::string inv, int sideCrafter, int sideBus,
    decltype(outFilter) outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(client), std::move(inv), sideCrafter, sideBus),
    outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessScatteringWorkingSet : ProcessSingleBlock {
  // Note: single input only.
  using Recipe = ::Recipe<>;
  std::string name;
  int eachSlotMaxInProc;
  std::vector<size_t> inSlots;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  ProcessScatteringWorkingSet(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, int eachSlotMaxInProc,
    decltype(inSlots) inSlots, decltype(outFilter) outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(client), std::move(inv), sideCrafter, sideBus),
    name(std::move(name)), eachSlotMaxInProc(eachSlotMaxInProc), inSlots(std::move(inSlots)),
    outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

inline std::vector<size_t> plantSowerInSlots() { return {6, 7, 8, 9, 10, 11, 12, 13, 14}; }

struct StockEntry {
  SharedItemFilter item;
  int toStock;
  bool allowBackup;
  StockEntry(SharedItemFilter item, int toStock, bool allowBackup)
    :item(std::move(item)), toStock(toStock), allowBackup(allowBackup) {}
};

struct ProcessHeterogeneousWorkingSet : ProcessSingleBlock {
  using Recipe = ::Recipe<>;
  std::string name;
  std::vector<StockEntry> stockList;
  int recipeMaxInProc;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  ProcessHeterogeneousWorkingSet(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, decltype(stockList) stockList,
    int recipeMaxInProc, decltype(outFilter) outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(client), std::move(inv), sideCrafter, sideBus),
    name(std::move(name)), stockList(std::move(stockList)), recipeMaxInProc(recipeMaxInProc),
    outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessInputless : ProcessSingleBlock {
  size_t sourceSlot;
  std::function<int()> needed;
  ProcessInputless(Factory &factory, std::string client, std::string inv,
    int sideCrafter, int sideBus, size_t sourceSlot, decltype(needed) needed)
    :ProcessSingleBlock(factory, std::move(client), std::move(inv), sideCrafter, sideBus),
    sourceSlot(sourceSlot), needed(std::move(needed)) {}
  SharedPromise<std::monostate> cycle() override;
  static std::function<int()> makeNeeded(Factory &factory, SharedItemFilter item, int toStock);
};

struct ProcessHeterogeneousInputless : ProcessSingleBlock {
  int needed;
  ProcessHeterogeneousInputless(Factory &factory, std::string client, std::string inv,
    int sideCrafter, int sideBus, int needed) :ProcessSingleBlock(factory, std::move(client),
    std::move(inv), sideCrafter, sideBus), needed(needed) {}
  SharedPromise<std::monostate> cycle() override;
};
