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

template<typename T>
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

template<typename T>
struct Recipe {
  int size = 1;
  SharedItemFilter out;
  std::vector<RecipeIn<T>> in;
};

template<typename T>
struct Demand {
  Recipe<T> *recipe;
  SharedItem out;
  std::vector<SharedItem> in;
  int outAvail;
  int inAvail;
};

struct Factory {
  Server &s;
private:
  std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
  std::string baseClient, baseInv;
  XNetCoord basePos;
  int minCycleTime;
  std::vector<XNetCoord> itemProviders;
  std::vector<std::pair<SharedItemFilter, int>> backups;
  std::vector<std::shared_ptr<Process>> processes;

  size_t currentCycleNum = 0;
  std::chrono::steady_clock::time_point cycleStartTime;
  std::shared_ptr<boost::asio::steady_timer> cycleDelayTimer;
  void endOfCycle();

  std::unordered_map<SharedItem, ItemInfo, SharedItemHash, SharedItemEqual> items;
  std::unordered_multimap<std::string, SharedItem> nameMap, labelMap;
  SharedPromise<std::monostate> updateItemProvider(const XNetCoord &pos);
  SharedPromise<std::monostate> updateItemProvidersAndBackupItems();
public:
  explicit Factory(Server &s, std::string baseClient, std::string baseInv, const XNetCoord &basePos, int minCycleTime)
    :s(s), baseClient(std::move(baseClient)), baseInv(std::move(baseInv)), basePos(basePos), minCycleTime(minCycleTime) {}
  void addItemProvider(const XNetCoord &pos);
  void addBackup(SharedItemFilter filter, int size);
  void addProcess(SharedProcess process);
  void start();

  SharedItem getItem(const ItemFilters::Base &filter);
  int getAvail(const SharedItem &item, bool allowBackup);
  void log(std::string msg, uint32_t color = 0xffffffu, float beep = -1.f);
  void extract(std::vector<SharedPromise<std::monostate>> &promises, const std::string &reason,
               const SharedItem &item, int size, const XNetCoord &to, int side);
  template<typename T>
  std::unique_ptr<Demand<T>> getDemand(const std::vector<Recipe<T>> &recipes) {
    std::unique_ptr<Demand<T>> result;
    float lastFulfillment = 2.f;
    for (auto &recipe : recipes) {
      auto demand(std::make_unique<Demand<T>>());
      demand->recipe = &recipe;
      if (recipe.out) demand->out = getItem(*recipe.out);
      demand.outAvail = getAvail(recipe.out, true);
      if (demand->outAvail >= recipe.size) continue;
      demand->inAvail = std::numeric_limits<int>::max();
      for (auto &in : recipe.in) {
        auto &inItem = demand->in.emplace_back(getItem(*in.item));
        demand->inAvail = std::min(demand->inAvail, getAvail(inItem) / in.size);
        if (!demand->inAvail) break;
      }
      if (!demand->inAvail) continue;
      float nowFulfillment = static_cast<float>(demand->outAvail) / recipe.size;
      if (nowFulfillment < lastFulfillment) {
        result = std::move(demand);
        lastFulfillment = nowFulfillment;
      }
    }
    return result;
  }
};
