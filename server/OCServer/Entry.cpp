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
    Factory factory(server, 1000, "center", "377", Actions::west);
    factory.addStorage(std::make_unique<StorageME>(factory, std::vector<AccessME>{{"center", "377", Actions::down, Actions::west}}));
    factory.addBackup(filterLabel("Fluxed Phyto-Gro"), 32);
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "center", "377", Actions::south, Actions::west,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 32)));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pulverizer", "center", "377", Actions::north, Actions::west, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gravel"), 32}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Sand"), 32}}, {{filterLabel("Gravel"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Charcoal"), 32}}, {{filterLabel("Charcoal"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Lead"), 32}}, {{filterLabel("Lead Ingot"), 1, {0}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manufactory", "center", "377", Actions::up, Actions::west, std::vector<size_t>{0},
      [](size_t slot, auto&&) { return 1 == slot; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Niter"), 32}}, {{filterLabel("Sandstone"), 1, {0}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "phyto", "center", "80c", Actions::north, Actions::east, std::vector<size_t>{0, 1},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel(u8"§eInferium Essence"), 4096}}, {{filterLabel("Fluxed Phyto-Gro"), 1, {0}},
          {filterLabel("Inferium Seeds"), 1, {1}}}, 16},
        {{{filterLabel("Wood Essence"), 32}}, {{filterLabel("Fluxed Phyto-Gro"), 1, {0}, true},
          {filterLabel("Wood Seeds"), 1, {1}}}, 16},
        {{{filterLabel("Lead Essence"), 32}}, {{filterLabel("Fluxed Phyto-Gro"), 1, {0}, true},
          {filterLabel("Lead Seeds"), 1, {1}}}, 16},
        {{{filterLabel("Diamond Essence"), 32}}, {{filterLabel("Fluxed Phyto-Gro"), 1, {0}},
          {filterLabel("Diamond Seeds"), 1, {1}}}, 16},
        {{{filterLabel("Seeds"), 32}}, {{filterLabel("Fluxed Phyto-Gro"), 1, {0}},
          {filterLabel("Seeds"), 1, {1}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "induction", "center", "80c", Actions::up, Actions::east, std::vector<size_t>{0, 1},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rich Slag"), 32}}, {{filterLabel("Hardened Glass"), 2, {0}},
          {filterLabel("Pulverized Lead"), 1, {1}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "80c", Actions::south, Actions::east, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "furnace", "center", "80c", Actions::west, Actions::east, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Charcoal"), 32}}, {{filterLabel("Oak Wood"), 1, {0}}}, 16}
      }));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorOut", "reactor", "010", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorIn", "reactor", "010", Actions::north, Actions::up,
      std::vector<StockEntry>{{filterLabel("Yellorium Ingot"), 16}}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessReactorHysteresis>(factory, "reactor", "reactor"));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "analogCrafter", "reactor", "010", Actions::east, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8}, [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sandstone"), 32}}, {{filterLabel("Sand"), 4, {0, 1, 3, 4}}}, 16},
        {{{filterLabel("Rich Phyto-Gro"), 32}}, {{filterLabel("Rich Slag"), 1, {0}},
          {filterLabel("Niter"), 1, {1}},
          {filterLabel("Pulverized Charcoal"), 1, {2}}}, 16},
        {{{filterLabel("Oak Wood"), 32}}, {{filterLabel("Wood Essence"), 3, {6, 7, 8}}}, 16},
        {{{filterLabel("Lead Ingot"), 32}}, {{filterLabel("Lead Essence"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}}, 8},
        {{{filterLabel("Diamond"), 32}}, {{filterLabel("Diamond Essence"), 9, {0, 1, 2, 3, 4, 5, 6, 7, 8}}}, 7}
      }));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
