#pragma once
#include <memory>
#include "Server.h"
#include "Actions.h"

struct ItemProvider {
  XNetCoord pos;
  int slot;
  SharedItemStack item;
  int nBackup = 0;
  int nReserved = 0;
};

struct Factory {
  Server &s;
private:
  std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
  std::string baseClient, baseInv;
  XNetCoord basePos;
  int minCycleTime;
  std::vector<XNetCoord> chests;

  size_t currentCycleNum = 0;
  std::chrono::steady_clock::time_point cycleStartTime;
  std::shared_ptr<boost::asio::steady_timer> cycleDelayTimer;
  void endOfCycle();

  std::vector<ItemProvider> itemProviders;
  std::unordered_multimap<std::string, size_t> nameMap, labelMap;

  SharedPromise<std::monostate> gatherChest(const XNetCoord &pos);
  SharedPromise<std::monostate> gatherChests();
public:
  explicit Factory(Server &s, std::string baseClient, std::string baseInv, const XNetCoord &basePos, int minCycleTime)
    :s(s), baseClient(std::move(baseClient)), baseInv(std::move(baseInv)), basePos(basePos), minCycleTime(minCycleTime) {}
  void addChest(const XNetCoord &pos);
  void cycle();
};
