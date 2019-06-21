#pragma once
#include <queue>
#include <unordered_set>
#include "Server.h"
#include "Actions.h"

struct Factory;

struct Process {
  Factory &factory;
  Process(Factory &factory) :factory(factory) {}
  virtual ~Process() = default;
  virtual SharedPromise<std::monostate> cycle() = 0;
};
using UniqueProcess = std::unique_ptr<Process>;

struct Provider {
  Factory &factory;
  int size;
  const int priority;
  Provider(Factory &factory, int size, int priority) :factory(factory), size(size), priority(priority) {}
  virtual ~Provider() = default;
  virtual SharedPromise<std::monostate> extract(int size, int slot) = 0 { this->size -= size; }
};
using UniqueProvider = std::unique_ptr<Provider>;
struct UniqueProviderLess { bool operator()(const UniqueProvider &x, const UniqueProvider &y) const { return x->priority < y->priority; } };

struct Storage {
  Factory &factory;
  Storage(Factory &factory) :factory(factory) {}
  virtual ~Storage() = default;
  virtual void endOfCycle() {};
  virtual SharedPromise<std::monostate> update() = 0;
  virtual std::optional<int> sinkPriority(const Item&) = 0;
  virtual std::pair<int, SharedPromise<std::monostate>> sink(const ItemStack&, size_t slot) = 0;
};
using UniqueStorage = std::unique_ptr<Storage>;

class ItemInfo {
  std::priority_queue<UniqueProvider, std::vector<UniqueProvider>, UniqueProviderLess> providers;
  int nAvail{}, nBackup{};
public:
  void addProvider(UniqueProvider provider);
  void backup(int size);
  int getAvail(bool allowBackup) const;
  void extract(std::vector<SharedPromise<std::monostate>> &promises, int size, int slot);
};

template<typename T = std::monostate>
struct RecipeIn {
  SharedItemFilter item;
  int size = 1;
  bool allowBackup = false;
  T data;

  template<typename ...Arg>
  RecipeIn(SharedItemFilter item, int size, bool allowBackup, Arg &&...xs)
      :item(std::move(item)), size(size), allowBackup(allowBackup), data(std::forward<Arg>(xs)...) {}
  RecipeIn(SharedItemFilter item, int size) :item(std::move(item)), size(size) {}
  explicit RecipeIn(SharedItemFilter item) :item(std::move(item)) {}
};

struct RecipeOut {
  SharedItemFilter item;
  int size;
};

template<typename T = std::monostate, typename U = std::monostate>
struct Recipe {
  std::vector<RecipeOut> out;
  std::vector<RecipeIn<U>> in;
  T data;
};

template<typename T, typename U>
struct Demand {
  const Recipe<T, U> &recipe;
  std::vector<SharedItem> in;
  int inAvail;
  float fullness;
  Demand(const Recipe<T, U> &recipe) :recipe(recipe) {}
};

struct BusWaitNode {
  SharedPromise<std::vector<size_t>> cont;
  bool allowPartial;
  size_t n;
};

struct Factory {
  Server &s;
  const std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
private:
  int minCycleTime;
  const std::string client, inv;
  std::vector<std::pair<SharedItemFilter, int>> backups;
  std::vector<UniqueProcess> processes;

  size_t currentCycleNum{};
  std::chrono::steady_clock::time_point cycleStartTime;
  std::shared_ptr<boost::asio::steady_timer> cycleDelayTimer;
  void endOfCycle();

  std::vector<UniqueStorage> storages;
  std::unordered_map<SharedItem, ItemInfo, SharedItemHash, SharedItemEqual> items;
  std::unordered_multimap<std::string, SharedItem> nameMap, labelMap;
  SharedPromise<std::monostate> updateAndBackupItems();
  void insertItem(std::vector<SharedPromise<std::monostate>> &promises, size_t slot, ItemStack stack);

  int sideBus;
  std::unordered_set<size_t> busAllocations;
  std::list<BusWaitNode> busWaitQueue;
  bool endOfCycleAfterBusUpdate{}, busEverUpdated{};
  enum class BusState { IDLE, RUNNING, RESTART } busState{BusState::IDLE};
  void doBusUpdate();
  void scheduleBusUpdate();
public:
  void log(std::string msg, uint32_t color = 0xffffffu, double beep = -1.f);
  ItemInfo &getOrAddItemInfo(const SharedItem &item);
  SharedItem getItem(const ItemFilters::Base &filter);
  int getAvail(const SharedItem &item, bool allowBackup);
  void extract(std::vector<SharedPromise<std::monostate>> &promises, const std::string &reason,
    const SharedItem &item, int size, int slot);
  template<typename T, typename U>
  std::vector<Demand<T, U>> getDemand(const std::vector<Recipe<T, U>> &recipes) {
    std::vector<Demand<T, U>> result;
    for (auto &recipe : recipes) {
      auto &demand(result.emplace_back(recipe));
      demand.fullness = 2.f;
      if (!recipe.out.empty()) {
        bool full{true};
        for (auto &i : recipe.out) {
          int outAvail = getAvail(getItem(*i.item), true);
          if (outAvail >= i.size)
            continue;
          full = false;
          float fullness{static_cast<float>(outAvail) / i.size};
          if (fullness < demand.fullness)
            demand.fullness = fullness;
        }
        if (full) {
          result.pop_back();
          continue;
        }
      }
      demand.inAvail = std::numeric_limits<int>::max();
      for (auto &in : recipe.in) {
        auto &inItem = demand.in.emplace_back(getItem(*in.item));
        demand.inAvail = std::min(demand.inAvail, getAvail(inItem, in.allowBackup) / in.size);
        if (!demand.inAvail)
          break;
      }
      if (!demand.inAvail) {
        result.pop_back();
        continue;
      }
    }
    std::sort(result.begin(), result.end(), [](const Demand<U, T> &x, const Demand<U, T> &y) {
      return x.fullness < y.fullness;
    });
    return result;
  }
  SharedPromise<std::vector<size_t>> busAllocate(bool allowPartial, size_t n);
  void busFree(const std::vector<size_t>&);

  Factory(Server &s, int minCycleTime, std::string client, std::string inv, int sideBus)
    :s(s), minCycleTime(minCycleTime), client(std::move(client)), inv(std::move(inv)), sideBus(sideBus) {}
  void addStorage(UniqueStorage storage) { storages.emplace_back(std::move(storage)); }
  void addBackup(SharedItemFilter filter, int size) { backups.emplace_back(std::move(filter), size); }
  void addProcess(UniqueProcess process) { processes.emplace_back(std::move(process)); }
  void start();
};
