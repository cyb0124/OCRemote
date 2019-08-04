#pragma once
#include "Factory.h"

struct ProcessSingleClient : Process {
  std::string name, client;
  ProcessSingleClient(Factory &factory, std::string name, std::string client)
    :Process(factory), name(std::move(name)), client(std::move(client)) {}
};

struct ProcessSingleBlock : ProcessSingleClient {
  std::string inv;
  int sideCrafter, sideBus;
  ProcessSingleBlock(Factory &factory, std::string name, std::string client, std::string inv, int sideCrafter, int sideBus)
    :ProcessSingleClient(factory, std::move(name), std::move(client)), inv(std::move(inv)), sideCrafter(sideCrafter), sideBus(sideBus) {}
  SharedPromise<std::monostate> processOutput(size_t slot, int size);
};

using OutFilter = std::function<bool(size_t slot, const ItemStack&)>;
inline bool outAll(size_t slot, const ItemStack&) { return true; }

struct ProcessSlotted : ProcessSingleBlock {
  using Recipe = ::Recipe<int, std::vector<size_t>>; // eachSlotMaxInProc, slots
  std::vector<size_t> inSlots;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessSlotted(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, decltype(inSlots) inSlots,
    OutFilter outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(name), std::move(client), std::move(inv), sideCrafter, sideBus),
    inSlots(std::move(inSlots)), outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct NonConsumableInfo {
  size_t storageSlot;
  size_t craftingGridSlot;
  NonConsumableInfo(size_t storageSlot, size_t craftingGridSlot)
    :storageSlot(storageSlot), craftingGridSlot(craftingGridSlot) {}
};

struct ProcessCraftingRobot : ProcessSingleClient {
  // Note: can't craft more than one stack at a time.
  // Slots: 1, 2, 3
  //        4, 5, 6
  //        7, 8, 9
  // NonConsumableSlots:
  //   4, 8, 12, 13, 14, 15
  //   with extra inventory: 17, 18, ...
  // (maxSets, nonConsumableInfo), slots
  using Recipe = ::Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>;
  int sideBus;
  std::vector<Recipe> recipes;
  size_t mapCraftingGridSlot(size_t slot);
  ProcessCraftingRobot(Factory &factory, std::string name, std::string client,
    int sideBus, std::vector<Recipe> recipes) :ProcessSingleClient(factory, std::move(name), std::move(client)),
    sideBus(sideBus), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessRFToolsControlWorkbench : ProcessSingleClient {
  using Recipe = ::Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>;
  std::string invIn, invOut;
  int sideBusIn, sideBusOut, sideNonConsumable;
  std::vector<Recipe> recipes;
  ProcessRFToolsControlWorkbench(Factory &factory, std::string name, std::string client, std::string invIn,
  std::string invOut, int sideBusIn, int sideBusOut, int sideNonConsumable, std::vector<Recipe> recipes)
    :ProcessSingleClient(factory, std::move(name), std::move(client)), invIn(std::move(invIn)),
    invOut(std::move(invOut)), sideBusIn(sideBusIn), sideBusOut(sideBusOut),
    sideNonConsumable(sideNonConsumable), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct StockEntry {
  SharedItemFilter item;
  int toStock;
  bool allowBackup;
  StockEntry(SharedItemFilter item, int toStock, bool allowBackup = false)
    :item(std::move(item)), toStock(toStock), allowBackup(allowBackup) {}
};

struct ProcessBuffered : ProcessSingleBlock {
  using Recipe = ::Recipe<int>; // maxInproc
  std::vector<StockEntry> stockList;
  int recipeMaxInProc;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessBuffered(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, decltype(stockList) stockList,
    int recipeMaxInProc, OutFilter outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(name), std::move(client), std::move(inv), sideCrafter, sideBus),
    stockList(std::move(stockList)), recipeMaxInProc(recipeMaxInProc),
    outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessScatteringWorkingSet : ProcessSingleBlock {
  // Note: single input only.
  using Recipe = ::Recipe<>;
  int eachSlotMaxInProc;
  std::vector<size_t> inSlots;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessScatteringWorkingSet(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, int eachSlotMaxInProc,
    decltype(inSlots) inSlots, OutFilter outFilter, decltype(recipes) recipes)
    :ProcessSingleBlock(factory, std::move(name), std::move(client), std::move(inv), sideCrafter, sideBus),
    eachSlotMaxInProc(eachSlotMaxInProc), inSlots(std::move(inSlots)),
    outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

inline std::vector<size_t> plantSowerInSlots() { return {6, 7, 8, 9, 10, 11, 12, 13, 14}; }

struct ProcessInputless : ProcessSingleBlock {
  size_t sourceSlot;
  std::function<int()> needed;
  ProcessInputless(Factory &factory, std::string name, std::string client, std::string inv,
    int sideCrafter, int sideBus, size_t sourceSlot, decltype(needed) needed)
    :ProcessSingleBlock(factory, std::move(name), std::move(client), std::move(inv), sideCrafter, sideBus),
    sourceSlot(sourceSlot), needed(std::move(needed)) {}
  SharedPromise<std::monostate> cycle() override;
  static std::function<int()> makeNeeded(Factory &factory, SharedItemFilter item, int toStock);
};

struct ProcessHeterogeneousInputless : ProcessSingleBlock {
  int needed;
  ProcessHeterogeneousInputless(Factory &factory, std::string name, std::string client, std::string inv, int sideCrafter, int sideBus, int needed)
    :ProcessSingleBlock(factory, std::move(name), std::move(client), std::move(inv), sideCrafter, sideBus), needed(needed) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessReactorHysteresis : ProcessSingleClient {
  std::string inv;
  int lowerBound, upperBound;
  ProcessReactorHysteresis(Factory &factory, std::string name, std::string client, std::string inv = "br_reactor", int lowerBound = 3000000, int upperBound = 7000000)
    :ProcessSingleClient(factory, std::move(name), std::move(client)), inv(std::move(inv)), lowerBound(lowerBound), upperBound(upperBound) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessReactorProportional : ProcessSingleClient {
  std::string inv;
  ProcessReactorProportional(Factory &factory, std::string name, std::string client, std::string inv = "br_reactor")
    :ProcessSingleClient(factory, std::move(name), std::move(client)), inv(std::move(inv)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessPlasticMixer : ProcessSingleClient {
  static const std::vector<std::string> colorMap;
  std::string inv;
  int needed;
  ProcessPlasticMixer(Factory &factory, std::string name, std::string client, int needed = 32, std::string inv = "plastic_mixer")
    :ProcessSingleClient(factory, std::move(name), std::move(client)), inv(std::move(inv)), needed(needed) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessRedstoneConditional : ProcessSingleClient {
  std::string inv;
  int side;
  bool logSkip;
  std::function<bool(int)> predicate;
  std::unique_ptr<Process> child;
  ProcessRedstoneConditional(Factory &factory, std::string name, std::string client,
    std::string inv, int side, bool logSkip, decltype(predicate) predicate, decltype(child) child)
    :ProcessSingleClient(factory, std::move(name), std::move(client)), inv(std::move(inv)),
    side(side), logSkip(logSkip), predicate(std::move(predicate)), child(std::move(child)) {}
  SharedPromise<std::monostate> cycle() override;
};
