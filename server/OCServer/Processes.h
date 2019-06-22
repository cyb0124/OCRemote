#pragma once
#include "Factory.h"

struct ProcessSingleBlock : Process {
  std::string client, inv;
  int sideCrafter, sideBus;
  ProcessSingleBlock(Factory &factory, std::string client, std::string inv, int sideCrafter, int sideBus)
    :Process(factory), client(std::move(client)), inv(std::move(inv)), sideCrafter(sideCrafter), sideBus(sideBus) {}
  SharedPromise<std::monostate> processOutput(int slot, int size);
};

struct ProcessSlotted : ProcessSingleBlock {
  using Recipe = ::Recipe<int, std::vector<size_t>>; // maxInProc, slots
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

struct ProcessWorkingSet : ProcessSingleBlock {
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

struct ProcessHeterogeneousWorkingSet : ProcessSingleBlock {
  using Recipe = ::Recipe<>;
  std::string name;
  std::vector<SharedItemFilter> stockList;
  int recipeMaxInProc;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  ProcessHeterogeneousWorkingSet(Factory &factory, std::string name, std::string client,
    std::string inv, int sideCrafter, int sideBus, decltype(stockList) stockList,
    int recipeMaxInProc, decltype(outFilter) outFilter, decltype(recipes) recipes);
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessInputless : ProcessSingleBlock {
  size_t sourceSlot;
  std::function<int()> needed;
  ProcessInputless(Factory &factory, std::string client, std::string inv,
    int sideCrafter, int sideBus, size_t sourceSlot, decltype(needed) needed);
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessHeterogeneousInputless : ProcessSingleBlock {
  int needed;
  ProcessHeterogeneousInputless(Factory &factory, std::string client, std::string inv,
    int sideCrafter, int sideBus, int needed);
  SharedPromise<std::monostate> cycle() override;
};
