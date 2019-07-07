#include <stdexcept>
#include <iostream>
#include <csignal>
#include "Server.h"
#include "Factory.h"
#include "Storages.h"
#include "Processes.h"

static void ignoreBrokenPipe() {
  #ifndef _WIN32
    struct sigaction sa{};
    sa.sa_handler = SIG_IGN;
      if (sigaction(SIGPIPE, &sa, nullptr))
        throw std::runtime_error("failed to ignore SIGPIPE");
  #endif
}

int main() {
  try {
    ignoreBrokenPipe();
    IOEnv io;
    Server server(io, 1847);
    Factory factory(server, 1000, "center", {{"center", "5cb", Actions::south}});
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "5cb", Actions::east, Actions::south));
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "a12", Actions::east, Actions::north));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "6ef", Actions::up, Actions::west, std::vector<SharedItemFilter>{
      filterLabel(u8"§eInferium Essence")
    }));

    // reactor
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "center"));

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "5cb", Actions::north, Actions::south,
      std::vector<StockEntry>{}, 0, outAll, std::vector<Recipe<int>>{}));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "center", "f80", "a12",
      Actions::east, Actions::north, -1, std::vector<Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Yellorium Ingot"), 64}}, {{filterLabel("Yellorium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Glowstone Dust"), 64}}, {{filterLabel("Glowstone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Redstone"), 64}}, {{filterLabel("Redstone Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Diamond Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 5, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Oak Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 2, 3}}}, {4, std::nullopt}},
        {{{filterLabel("Sandstone"), 16}}, {{filterLabel("Sand"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Rich Phyto-Gro"), 16}}, {{filterLabel("Pulverized Charcoal"), 1, {1}},
          {filterLabel("Niter"), 1, {2}}, {filterLabel("Rich Slag"), 1, {3}}}, {4, std::nullopt}},
        {{{filterLabel("Clock"), 16}}, {{filterLabel("Redstone"), 1, {5}},
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}}
      }));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "cobbleGen", "center", "6ef", Actions::south, Actions::west,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 640)));

    // snowballGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "snowballGen", "center", "5cb", Actions::up, Actions::south,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Snowball"), 64)));

    // manufactory
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "manufactory", "center", "5cb", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Sand"), 640}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Niter"), 16}}, {{filterLabel("Sandstone"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Charcoal"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "furnace", "center", "a12", Actions::west, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Glass"), 64}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Stone"), 64}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Oak Wood"), 1}}, INT_MAX}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "a12", Actions::south, Actions::north,
      std::vector<StockEntry>{
        {filterLabel("Fluxed Phyto-Gro"), 16},
        {filterLabel("Yellorium Ingot"), 16}
      }, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§eInferium Essence"), 4096}}, {{filterName("mysticalagriculture:tier5_inferium_seeds"), 1}}, INT_MAX},
        {{{filterLabel("Yellorium Essence"), 16}}, {{filterLabel("Yellorium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Essence"), 16}}, {{filterLabel("Glowstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 16}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Essence"), 16}}, {{filterLabel("Diamond Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Essence"), 16}}, {{filterLabel("Cobalt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Wood Essence"), 16}}, {{filterLabel("Wood Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Ender Pearl"), 16}}, {{filterLabel("Ender Lilly"), 1}}, INT_MAX},
        {{{filterLabel("Seeds"), 16}}, {{filterLabel("Seeds"), 1}}, INT_MAX}
      }));

    // sandInduction
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sandInduction", "center", "f80", Actions::west, Actions::east,
      std::vector<StockEntry>{{filterLabel("Sand"), 16}}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Rich Slag"), 16}}, {{filterLabel("Clock"), 1}}, INT_MAX}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "f80", Actions::up, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
