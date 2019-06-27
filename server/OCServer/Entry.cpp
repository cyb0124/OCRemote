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
    Factory factory(server, 1000, "center", {{"center", "377", Actions::west}, {"reactor", "010", Actions::up}});
    factory.addStorage(std::make_unique<StorageME>(factory, std::vector<AccessME>{{"center", "377", Actions::down, Actions::west}}));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "center", "377", Actions::south, Actions::west,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 64)));

    // pulverizer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pulverizer", "center", "377", Actions::north, Actions::west, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gravel"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Sand"), 64}}, {{filterLabel("Gravel"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Charcoal"), 64}}, {{filterLabel("Charcoal"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Lead"), 64}}, {{filterLabel("Lead Ingot"), 1, {0}}}, 16}
      }));

    // manufactory
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manufactory", "center", "377", Actions::up, Actions::west, std::vector<size_t>{0},
      [](size_t slot, auto&&) { return 1 == slot; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Niter"), 64}}, {{filterLabel("Sandstone"), 1, {0}}}, 16}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "phyto", "center", "80c", Actions::north, Actions::east,
      std::vector<StockEntry>{{filterLabel("Fluxed Phyto-Gro"), 64}}, 16, nullptr,
      std::vector<Recipe<>>{
        {{{filterLabel(u8"§eInferium Essence"), 4096}}, {{filterName("mysticalagriculture:tier5_inferium_seeds"), 1}}},
        {{{filterLabel("Wood Essence"), 64}}, {{filterLabel("Wood Seeds"), 1}}},
        {{{filterLabel("Lead Essence"), 64}}, {{filterLabel("Lead Seeds"), 1}}},
        {{{filterLabel("Diamond Essence"), 64}}, {{filterLabel("Diamond Seeds"), 1}}},
        {{{filterLabel("Redstone Essence"), 64}}, {{filterLabel("Redstone Seeds"), 1}}},
        {{{filterLabel("Yellorium Essence"), 64}}, {{filterLabel("Yellorium Seeds"), 1}}},
        {{{filterLabel("Certus Quartz Essence"), 64}}, {{filterLabel("Certus Quartz Seeds"), 1}}},
        {{{filterLabel("Seeds"), 64}}, {{filterLabel("Seeds"), 1}}}
      }));

    // output
    factory.addProcess(std::make_unique<ProcessWorkingSet>(factory, "center", "ee2", Actions::east, Actions::west,
      [](auto&&...) { return true; }, std::vector<Recipe<std::pair<std::string, int>>>{}));

    // induction
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "induction", "center", "80c", Actions::up, Actions::east, std::vector<size_t>{0, 1},
      nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rich Slag"), 64}}, {{filterLabel("Hardened Glass"), 2, {0}},
          {filterLabel("Pulverized Lead"), 1, {1}}}, 16}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "80c", Actions::south, Actions::east, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 256}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "furnace", "center", "80c", Actions::west, Actions::east, std::vector<size_t>{0},
      nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Oak Wood"), 1, {0}}}, 16}
      }));

    // reactor
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorOut", "reactor", "010", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorIn", "reactor", "010", Actions::north, Actions::up,
      std::vector<StockEntry>{{filterLabel("Yellorium Ingot"), 16}}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "reactor"));

    // analogCrafter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "analogCrafter", "reactor", "010", Actions::east, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8}, [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sandstone"), 64}}, {{filterLabel("Sand"), 4, {0, 1, 3, 4}}}, 16},
        {{{filterLabel("Rich Phyto-Gro"), 64}}, {{filterLabel("Rich Slag"), 1, {0}},
          {filterLabel("Niter"), 1, {1}},
          {filterLabel("Pulverized Charcoal"), 1, {2}}}, 16},
        {{{filterLabel("Oak Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {6, 7, 8}}}, 16},
        {{{filterLabel("Lead Ingot"), 64}}, {{filterLabel("Lead Essence"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}}, 8},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Diamond Essence"), 9, {0, 1, 2, 3, 4, 5, 6, 7, 8}}}, 7},
        {{{filterLabel("Redstone"), 64}}, {{filterLabel("Redstone Essence"), 9, {0, 1, 2, 3, 4, 5, 6, 7, 8}}}, 7},
        {{{filterLabel("Yellorium Ingot"), 64}}, {{filterLabel("Yellorium Essence"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}}, 8},
        {{{filterLabel("Certus Quartz Crystal"), 64}}, {{filterLabel("Certus Quartz Essence"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}}, 8}
      }));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
