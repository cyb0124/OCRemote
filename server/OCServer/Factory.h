#pragma once
#include <memory>
#include "Server.h"
#include "Actions.h"

struct Factory;

struct Process : std::enable_shared_from_this<Process> {
  virtual ~Process() = default;
  virtual SharedPromise<std::monostate> cycle(Factory&) = 0;
};
using SharedProcess = std::shared_ptr<Process>;

struct ItemProvider {
  XNetCoord pos;
  int slot;
  int size;
};

class ItemInfo {
  std::list<ItemProvider> providers;
  int nBackup = 0;
public:
  void addProvider(const XNetCoord &pos, int slot, int size);
  void backup(int size);
  int getAvail(bool allowBackup) const;
  ItemProvider extractSome(int size);
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

template<typename RecipeData = std::monostate, typename InData = std::monostate>
struct Recipe {
  std::vector<RecipeOut> out;
  std::vector<RecipeIn<InData>> in;
  RecipeData data;
};

template<typename U, typename T>
struct Demand {
  const Recipe<U, T> *recipe;
  std::vector<SharedItem> in;
  int inAvail;
  float fullness;
};

struct Factory {
  Server &s;
  const std::string baseClient, baseInv;
  const XNetCoord basePos;
private:
  std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
  int minCycleTime;
  std::vector<XNetCoord> chests;
  std::vector<std::pair<SharedItemFilter, int>> backups;
  std::vector<std::shared_ptr<Process>> processes;

  size_t currentCycleNum = 0;
  std::chrono::steady_clock::time_point cycleStartTime;
  std::shared_ptr<boost::asio::steady_timer> cycleDelayTimer;
  void endOfCycle();

  std::unordered_map<SharedItem, ItemInfo, SharedItemHash, SharedItemEqual> items;
  std::unordered_multimap<std::string, SharedItem> nameMap, labelMap;
  ItemInfo &getOrAddInfo(const SharedItem &item);
  SharedPromise<std::monostate> updateChest(const XNetCoord &pos);
  SharedPromise<std::monostate> updateAndBackupItems();
public:
  explicit Factory(Server &s, std::string baseClient, std::string baseInv, const XNetCoord &basePos, int minCycleTime)
    :s(s), baseClient(std::move(baseClient)), baseInv(std::move(baseInv)), basePos(basePos), minCycleTime(minCycleTime) {}
  void addChest(const XNetCoord &pos);
  void addBackup(SharedItemFilter filter, int size);
  void addProcess(SharedProcess process);
  void start();

  SharedItem getItem(const ItemFilters::Base &filter);
  int getAvail(const SharedItem &item, bool allowBackup);
  void log(std::string msg, uint32_t color = 0xffffffu, float beep = -1.f);
  void extract(std::vector<SharedPromise<std::monostate>> &promises, const std::string &reason,
               const SharedItem &item, int size, const XNetCoord &to, int side);
  template<typename U, typename T>
  std::vector<Demand<U, T>> getDemand(const std::vector<Recipe<U, T>> &recipes) {
    std::vector<Demand<U, T>> result;
    for (auto &recipe : recipes) {
      auto &demand(result.emplace_back());
      demand.recipe = &recipe;
      demand.fullness = 2.f;
      if (!recipe.out.empty()) {
        bool full = true;
        for (auto &i : recipe.out) {
          int outAvail = getAvail(getItem(*i.item), true);
          if (outAvail >= i.size)
            continue;
          full = false;
          float fullness = static_cast<float>(outAvail) / i.size;
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
};
