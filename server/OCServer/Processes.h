#pragma once
#include "Factory.h"

template<typename T>
struct ProcessAccess : Process {
  std::vector<T> accesses;
  ProcessAccess(Factory &factory, std::string name, decltype(accesses) accesses)
    :Process(factory, std::move(name)), accesses(std::move(accesses)) {}
};

struct ProcessAccessInv : ProcessAccess<AccessInv> {
  using ProcessAccess<AccessInv>::ProcessAccess;
  SharedPromise<std::monostate> processOutput(size_t slot, int size);
};

using OutFilter = std::function<bool(size_t slot, const ItemStack&)>;
using SlotFilter = std::function<bool(size_t slot)>;
inline bool outAll(size_t slot, const ItemStack&) { return true; }

struct ProcessSlotted : ProcessAccessInv {
  using Recipe = ::Recipe<int, std::vector<size_t>>; // eachSlotMaxInProc, slots
  std::vector<size_t> inSlots;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessSlotted(Factory &factory, std::string name, decltype(accesses) accesses,
    decltype(inSlots) inSlots, OutFilter outFilter, decltype(recipes) recipes)
    :ProcessAccessInv(factory, std::move(name), std::move(accesses)),
    inSlots(std::move(inSlots)), outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct NonConsumableInfo {
  size_t storageSlot;
  size_t craftingGridSlot;
  NonConsumableInfo(size_t storageSlot, size_t craftingGridSlot)
    :storageSlot(storageSlot), craftingGridSlot(craftingGridSlot) {}
};

struct AccessCraftingRobot : Access {
  int sideBus;
  AccessCraftingRobot(std::string client, int sideBus)
    :Access(std::move(client)), sideBus(sideBus) {}
};

struct ProcessCraftingRobot : ProcessAccess<AccessCraftingRobot> {
  // Note: can't craft more than one stack at a time.
  // Slots: 1, 2, 3
  //        4, 5, 6
  //        7, 8, 9
  // NonConsumableSlots:
  //   4, 8, 12, 13, 14, 15
  //   with extra inventory: 17, 18, ...
  // (maxSets, nonConsumableInfo), slots
  using Recipe = ::Recipe<std::pair<int, std::vector<NonConsumableInfo>>, std::vector<size_t>>;
  std::vector<Recipe> recipes;
  size_t mapCraftingGridSlot(size_t slot);
  ProcessCraftingRobot(Factory &factory, std::string name, decltype(accesses) accesses, std::vector<Recipe> recipes)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct AccessRFToolsControlWorkbench : Access {
  std::string addrIn, addrOut;
  int sideBusIn, sideBusOut, sideNonConsumable;
  AccessRFToolsControlWorkbench(std::string client, std::string addrIn, std::string addrOut,
    int sideBusIn, int sideBusOut, int sideNonConsumable)
    :Access(std::move(client)), addrIn(std::move(addrIn)), addrOut(std::move(addrOut)),
    sideBusIn(sideBusIn), sideBusOut(sideBusOut), sideNonConsumable(sideNonConsumable) {}
};

struct ProcessRFToolsControlWorkbench : ProcessAccess<AccessRFToolsControlWorkbench> {
  using Recipe = ::Recipe<std::pair<int, std::vector<NonConsumableInfo>>, std::vector<size_t>>;
  std::vector<Recipe> recipes;
  ProcessRFToolsControlWorkbench(Factory &factory, std::string name, decltype(accesses) accesses, std::vector<Recipe> recipes)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct StockEntry {
  SharedItemFilter item;
  int toStock;
  bool allowBackup;
  StockEntry(SharedItemFilter item, int toStock, bool allowBackup = false)
    :item(std::move(item)), toStock(toStock), allowBackup(allowBackup) {}
};

struct ProcessBuffered : ProcessAccessInv {
  using Recipe = ::Recipe<int>; // maxInproc
  std::vector<StockEntry> stockList;
  int recipeMaxInProc;
  SlotFilter slotFilter;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessBuffered(Factory &factory, std::string name, decltype(accesses) accesses, decltype(stockList) stockList,
    int recipeMaxInProc, SlotFilter slotFilter, OutFilter outFilter, decltype(recipes) recipes)
    :ProcessAccessInv(factory, std::move(name), std::move(accesses)), stockList(std::move(stockList)),
    recipeMaxInProc(recipeMaxInProc), slotFilter(std::move(slotFilter)), outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessScatteringWorkingSet : ProcessAccessInv {
  // Note: single input only.
  using Recipe = ::Recipe<>;
  int eachSlotMaxInProc;
  std::vector<size_t> inSlots;
  OutFilter outFilter;
  std::vector<Recipe> recipes;
  ProcessScatteringWorkingSet(Factory &factory, std::string name, decltype(accesses) accesses,
    int eachSlotMaxInProc, decltype(inSlots) inSlots, OutFilter outFilter, decltype(recipes) recipes)
    :ProcessAccessInv(factory, std::move(name), std::move(accesses)), eachSlotMaxInProc(eachSlotMaxInProc),
    inSlots(std::move(inSlots)), outFilter(std::move(outFilter)), recipes(std::move(recipes)) {}
  SharedPromise<std::monostate> cycle() override;
};

inline std::vector<size_t> plantSowerInSlots() { return {6, 7, 8, 9, 10, 11, 12, 13, 14}; }

struct InputlessEntry {
  SharedItemFilter item;
  int needed;
};

struct ProcessInputless : ProcessAccessInv {
  SlotFilter slotFilter;
  std::vector<InputlessEntry> entries;
  ProcessInputless(Factory &factory, std::string name, decltype(accesses) accesses, SlotFilter slotFilter, decltype(entries) entries)
    :ProcessAccessInv(factory, std::move(name), std::move(accesses)), slotFilter(std::move(slotFilter)), entries(std::move(entries)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessReactor : ProcessAccess<AccessAddr> {
  int cyaniteNeeded;
  bool hasTurbine;
  // Typical address: br_reactor
  ProcessReactor(Factory &factory, std::string name, decltype(accesses) accesses, int cyaniteNeeded, bool hasTurbine)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), cyaniteNeeded(cyaniteNeeded), hasTurbine(hasTurbine) {}
  SharedPromise<double> getPV();
};

struct ProcessReactorHysteresis : ProcessReactor {
  double lowerBound, upperBound;
  int wasOn;
  ProcessReactorHysteresis(Factory &factory, std::string name, decltype(accesses) accesses,
    int cyaniteNeeded = 0, bool hasTurbine = false, double lowerBound = 0.3, double upperBound = 0.7)
    :ProcessReactor(factory, std::move(name), std::move(accesses), cyaniteNeeded, hasTurbine),
    lowerBound(lowerBound), upperBound(upperBound), wasOn(-1) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessReactorProportional : ProcessReactor {
  int prev;
  ProcessReactorProportional(Factory &factory, std::string name, decltype(accesses) accesses, int cyaniteNeeded = 0, bool hasTurbine = false)
    :ProcessReactor(factory, std::move(name), std::move(accesses), cyaniteNeeded, hasTurbine), prev(-1) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessReactorPID : ProcessReactor {
  double kP, kI, kD;
  bool isInit;
  std::chrono::time_point<std::chrono::steady_clock> prevT;
  double prevE, accum;
  int prevOut;
  ProcessReactorPID(Factory &factory, std::string name, decltype(accesses) accesses, int cyaniteNeeded = 0, bool hasTurbine = false,
    double kP = 1, double kI = 0.01, double kD = 0, double initAccum = 0)
    :ProcessReactor(factory, std::move(name), std::move(accesses), cyaniteNeeded, hasTurbine),
    kP(kP), kI(kP * kI), kD(kP * kD), isInit(true), accum(initAccum), prevOut(-1) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessPlasticMixer : ProcessAccess<AccessAddr> {
  static const std::vector<std::string> colorMap;
  int needed, prev;
  // Typical address: plastic_mixer
  ProcessPlasticMixer(Factory &factory, std::string name, decltype(accesses) accesses, int needed = 32)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), needed(needed), prev(-1) {}
  SharedPromise<std::monostate> cycle() override;
};

struct AccessRedstone : AccessAddr {
  int side;
  AccessRedstone(std::string client, std::string addr, int side)
    :AccessAddr(std::move(client), std::move(addr)), side(side) {}
};

struct ProcessRedstoneConditional : ProcessAccess<AccessRedstone> {
  bool logSkip;
  std::function<bool(int)> predicate;
  std::function<bool()> precondition;
  std::unique_ptr<Process> child;
  ProcessRedstoneConditional(Factory &factory, std::string name, decltype(accesses) accesses,
    bool logSkip, decltype(precondition) precondition, decltype(predicate) predicate, decltype(child) child)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), logSkip(logSkip),
    precondition(std::move(precondition)), predicate(std::move(predicate)), child(std::move(child)) {}
  SharedPromise<std::monostate> cycle() override;
};

struct ProcessRedstoneEmitter : ProcessAccess<AccessRedstone> {
  int prevValue;
  std::function<int()> valueFn;
  ProcessRedstoneEmitter(Factory &factory, std::string name, decltype(accesses) accesses, decltype(valueFn) valueFn)
    :ProcessAccess(factory, std::move(name), std::move(accesses)), prevValue(-1), valueFn(std::move(valueFn)) {}
  SharedPromise<std::monostate> cycle() override;
  static std::function<int()> makeNeeded(Factory &factory, std::string name, SharedItemFilter item, int toStock);
};

struct FluxNetworkOutput {
  std::string name;
  std::vector<AccessRedstone> accesses;
  std::function<int(double)> valueFn;
};

struct ProcessFluxNetwork : ProcessAccess<AccessAddr> {
  std::vector<std::unique_ptr<ProcessRedstoneEmitter>> outputs;
  double lastEnergy;
  // Typical address: flux_controller
  ProcessFluxNetwork(Factory &factory, std::string name, decltype(accesses) accesses, std::vector<FluxNetworkOutput> outputs)
    :ProcessAccess(factory, std::move(name), std::move(accesses)) {
    for (auto &output : outputs)
      this->outputs.emplace_back(std::make_unique<ProcessRedstoneEmitter>(factory, std::move(output.name), std::move(output.accesses),
        [this, valueFn(std::move(output.valueFn))]() {
          return valueFn(lastEnergy);
        }));
  }

  SharedPromise<std::monostate> cycle() override;
};
