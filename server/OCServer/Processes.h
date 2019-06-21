#pragma once
#include "Factory.h"

struct ProcessSlotted : Process {
  using Recipe = ::Recipe<std::monostate, std::vector<size_t>>;
private:
  std::string name, client, inv;
  int sideCrafter, sideBus;
  std::vector<size_t> inSlots;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  std::function<bool()> fnSkip;
public:
  ProcessSlotted(Factory &factory, std::string name, std::string client, std::string inv,
    int sideCrafter, int sideBus, decltype(inSlots) inSlots, decltype(outFilter) outFilter,
    decltype(recipes) recipes, decltype(fnSkip) fnSkip);
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessWorkingSet : Process {
  using Recipe = ::Recipe<std::string>;
private:
  std::string client, inv;
  int sideWorkingSet, sideBus;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  std::function<bool()> fnSkip;
public:
  ProcessWorkingSet(Factory &factory, std::string client, std::string inv,
    int sideWorkingSet, int sideBus, decltype(outFilter) outFilter,
    decltype(recipes) recipes, decltype(fnSkip) fnSkip);
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessHeterogeneousWorkingSet : Process {
  using Recipe = ::Recipe<>;
private:
  std::string name, client, inv;
  int sideWorkingSet, sideBus;
  std::vector<SharedItemFilter> stockList;
  int recipeMaxInProc;
  std::function<bool(size_t slot, const ItemStack&)> outFilter;
  std::vector<Recipe> recipes;
  std::function<bool()> fnSkip;
public:
  ProcessHeterogeneousWorkingSet(Factory &factory, std::string name, std::string client, std::string inv,
    int sideWorkingSet, int sideBus, decltype(stockList) stockList, int recipeMaxInProc,
    decltype(outFilter) outFilter, decltype(recipes) recipes, decltype(fnSkip) fnSkip);
  SharedPromise<std::monostate> cycle() override;
};

class ProcessInputless : public Process {
  std::string client, inv;
  int sideSource, sideBus;
  size_t sourceSlot;
  std::function<int()> needed;
public:
  ProcessInputless(Factory &factory, std::string client, std::string inv,
    int sideSource, int sideBus, size_t sourceSlot, decltype(needed) needed);
  SharedPromise<std::monostate> cycle() override;
};

class ProcessHeterogeneousInputless : public Process {
  std::string client, inv;
  int sideSource, sideBus, needed;
public:
  ProcessHeterogeneousInputless(Factory &factory, std::string client, std::string inv,
    int sideSource, int sideBus, int needed);
  SharedPromise<std::monostate> cycle() override;
};
