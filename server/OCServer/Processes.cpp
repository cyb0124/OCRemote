#include "Processes.h"

ProcessSlotted::ProcessSlotted(
  Factory &factory, std::string name, std::string client, std::string inv,
  int sideCrafter, int sideBus, decltype(inSlots) inSlots, decltype(outFilter) outFilter,
  decltype(recipes) recipes, decltype(fnSkip) fnSkip)
  :Process(factory), name(std::move(name)), client(std::move(client)), inv(std::move(inv)),
  sideCrafter(sideCrafter), sideBus(sideBus), inSlots(std::move(inSlots)), outFilter(std::move(outFilter)),
  recipes(std::move(recipes)), fnSkip(std::move(fnSkip)) {}

ProcessWorkingSet::ProcessWorkingSet(
  Factory &factory, std::string client, std::string inv,
  int sideWorkingSet, int sideBus, decltype(outFilter) outFilter,
  decltype(recipes) recipes, decltype(fnSkip) fnSkip)
  :Process(factory), client(std::move(client)), inv(std::move(inv)),
  sideWorkingSet(sideWorkingSet), sideBus(sideBus), outFilter(std::move(outFilter)),
  recipes(std::move(recipes)), fnSkip(std::move(fnSkip)) {}

ProcessHeterogeneousWorkingSet::ProcessHeterogeneousWorkingSet(
  Factory &factory, std::string name, std::string client, std::string inv,
  int sideWorkingSet, int sideBus, decltype(stockList) stockList, int recipeMaxInProc,
  decltype(outFilter) outFilter, decltype(recipes) recipes, decltype(fnSkip) fnSkip)
  :Process(factory), name(std::move(name)), client(std::move(client)), inv(std::move(inv)),
  sideWorkingSet(sideWorkingSet), sideBus(sideBus), stockList(std::move(stockList)), recipeMaxInProc(recipeMaxInProc),
  outFilter(std::move(outFilter)), recipes(std::move(recipes)), fnSkip(std::move(fnSkip)) {}

ProcessInputless::ProcessInputless(
  Factory &factory, std::string client, std::string inv,
  int sideSource, int sideBus, size_t sourceSlot, decltype(needed) needed)
  :Process(factory), client(std::move(client)), inv(std::move(inv)),
  sideSource(sideSource), sideBus(sideBus), sourceSlot(sourceSlot), needed(std::move(needed)) {}

ProcessHeterogeneousInputless::ProcessHeterogeneousInputless(
  Factory &factory, std::string client, std::string inv,
  int sideSource, int sideBus, int needed)
  :Process(factory), client(std::move(client)), inv(std::move(inv)),
  sideSource(sideSource), sideBus(sideBus), needed(needed) {}
