#ifndef _FACTORY_H_
#define _FACTORY_H_
#include <queue>
#include <unordered_set>
#include "Server.h"
#include "Actions.h"

struct Factory;

struct AccessAddr : Access {
  std::string addr;
  AccessAddr(std::string client, std::string addr)
    :Access(std::move(client)), addr(std::move(addr)) {}
};

struct AccessBus : AccessAddr {
  int sideBus;
  AccessBus(std::string client, std::string addr, int sideBus)
    :AccessAddr(std::move(client), std::move(addr)), sideBus(sideBus) {}
};

struct AccessInv : AccessBus {
  int sideInv;
  AccessInv(std::string client, std::string addr, int sideInv, int sideBus)
    :AccessBus(std::move(client), std::move(addr), sideBus), sideInv(sideInv) {}
};

struct Process {
  Factory &factory;
  std::string name;
  Process(Factory &factory, std::string name)
    :factory(factory), name(std::move(name)) {}
  virtual ~Process() = default;
  virtual SharedPromise<std::monostate> cycle() = 0;
};
using UniqueProcess = std::unique_ptr<Process>;

struct Provider {
  Factory &factory;
  int avail;
  const int priority;
  Provider(Factory &factory, int avail, int priority) :factory(factory), avail(avail), priority(priority) {}
  virtual ~Provider() = default;
  virtual SharedPromise<std::monostate> extract(int size, size_t slot) = 0;
};
using SharedProvider = std::shared_ptr<Provider>;
struct SharedProviderLess { bool operator()(const SharedProvider &x, const SharedProvider &y) const { return x->priority < y->priority; } };

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

struct Reservation {
  std::vector<std::pair<SharedProvider, int>> providers;
  SharedPromise<std::monostate> extract(size_t slot) const;
};

class ItemInfo {
  std::priority_queue<SharedProvider, std::vector<SharedProvider>, SharedProviderLess> providers;
  int nAvail{}, nBackup{};
public:
  void addProvider(SharedProvider provider);
  void backup(int size);
  int getAvail(bool allowBackup) const;
  Reservation reserve(int size);
};

template<typename T = std::monostate>
struct RecipeIn {
  SharedItemFilter item;
  int size{1}, extraBackup{};
  bool allowBackup{};
  T data;

  RecipeIn(SharedItemFilter item, int size, T data, bool allowBackup = false, int extraBackup = 0)
      :item(std::move(item)), size(size), extraBackup(extraBackup), allowBackup(allowBackup), data(std::move(data)) {}
  RecipeIn(SharedItemFilter item, int size) :item(std::move(item)), size(size) {}
  RecipeIn(SharedItemFilter item) :item(std::move(item)) {}
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
  const Recipe<T, U> *recipe;
  std::vector<SharedItem> in;
  int inAvail;
  float fullness;
  Demand(const Recipe<T, U> *recipe) :recipe(recipe) {}
};

struct InputAvailInfo {
  int avail, needed;
  bool allowBackup;
  InputAvailInfo() :needed() {}
};

struct Factory {
  Server &s;
  const std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
private:
  int minCycleTime;
  std::vector<std::string> logClients;
  std::vector<std::pair<SharedItemFilter, int>> backups;
  std::vector<UniqueProcess> processes;

  size_t nCycles{};
  std::chrono::steady_clock::time_point cycleStartTime{std::chrono::steady_clock::now()};
  std::shared_ptr<boost::asio::steady_timer> cycleDelayTimer;
  void endOfCycle();

  std::vector<UniqueStorage> storages;
  std::unordered_map<SharedItem, ItemInfo, SharedItemHash, SharedItemEqual> items;
  std::unordered_multimap<std::string, SharedItem> nameMap, labelMap;
  SharedPromise<std::monostate> updateAndBackupItems();
  void insertItem(std::vector<SharedPromise<std::monostate>> &promises, size_t slot, ItemStack stack);

  std::vector<AccessBus> busAccesses;
  std::unordered_set<size_t> busAllocations;
  std::list<SharedPromise<size_t>> busWaitQueue;
  std::vector<size_t> busFreeQueue;
  bool endOfCycleAfterBusUpdate{};
  size_t nBusUpdates{};
  enum class BusState { IDLE, RUNNING, RESTART } busState{BusState::IDLE};
  void doBusUpdate();
  void scheduleBusUpdate();
public:
  void log(std::string msg, uint32_t color = 0xffffffu, double beep = -1.f);
  ItemInfo &getOrAddItemInfo(const SharedItem &item);
  SharedItem getItem(const ItemFilters::Base &filter);
  int getAvail(const SharedItem &item, bool allowBackup);
  Reservation reserve(const std::string &reason, const SharedItem &item, int size);
  template<typename T, typename U>
  void resolveRecipeInputs(const Recipe<T, U> &recipe, Demand<T, U> &demand, bool clipToMaxStackSize) {
    demand.inAvail = std::numeric_limits<int>::max();
    std::unordered_map<SharedItem, InputAvailInfo, SharedItemHash, SharedItemEqual> avails;
    for (auto &in : recipe.in) {
      auto &inItem(demand.in.emplace_back(getItem(*in.item)));
      if (!inItem) {
        demand.inAvail = 0;
        continue;
      }
      auto itr(avails.try_emplace(inItem));
      auto &info(itr.first->second);
      if (itr.second || info.allowBackup && !in.allowBackup) {
        info.avail = std::max(0, getAvail(inItem, in.allowBackup) - in.extraBackup);
        info.allowBackup = in.allowBackup;
      }
      info.needed += in.size;
      if (clipToMaxStackSize) {
        demand.inAvail = std::min(demand.inAvail, inItem->maxSize / in.size);
      }
    }
    for (auto &entry : avails) {
      demand.inAvail = std::min(demand.inAvail, entry.second.avail / entry.second.needed);
    }
  }
  template<typename T, typename U>
  std::vector<Demand<T, U>> getDemand(const std::vector<Recipe<T, U>> &recipes, bool clipToMaxStackSize = true) {
    std::vector<Demand<T, U>> result;
    for (auto &recipe : recipes) {
      auto &demand(result.emplace_back(&recipe));
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
      resolveRecipeInputs(recipe, demand, clipToMaxStackSize);
      if (!demand.inAvail) {
        result.pop_back();
        continue;
      }
    }
    std::sort(result.begin(), result.end(), [](const Demand<T, U> &x, const Demand<T, U> &y) {
      return x.fullness < y.fullness;
    });
    return result;
  }
  SharedPromise<size_t> busAllocate();
  void busFree(size_t slot, bool hasItem);
  void busFree(const std::vector<size_t> &slots, bool hasItem);

  Factory(Server &s, int minCycleTime, decltype(logClients) logClients, decltype(busAccesses) busAccesses)
    :s(s), minCycleTime(minCycleTime), logClients(std::move(logClients)), busAccesses(std::move(busAccesses)) {}
  void addStorage(UniqueStorage storage) { storages.emplace_back(std::move(storage)); }
  void addBackup(SharedItemFilter filter, int size) { backups.emplace_back(std::move(filter), size); }
  void addProcess(UniqueProcess process) { processes.emplace_back(std::move(process)); }
  void start();
};

#endif
