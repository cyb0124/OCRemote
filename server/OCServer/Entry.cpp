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
    Factory factory(server, 1000, "center", {
      {"center", "5cb", Actions::south},
      {"crafter", "inventory_controller", Actions::front}});
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "5cb", Actions::east, Actions::south));
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "a12", Actions::east, Actions::north));

    // output
    factory.addProcess(std::make_unique<ProcessWorkingSet>(factory, "center", "5cb", Actions::north, Actions::south,
      [](auto&&...) { return true; }, std::vector<Recipe<std::pair<std::string, int>>>{}));

    // analogCrafter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "analogCrafter", "center", "a12", Actions::south, Actions::north,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sandstone"), 64}}, {{filterLabel("Sand"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Tin Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Iron Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Gold Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Zinc Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Lead Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Copper Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Silver Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Osmium Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Nickel Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Aluminum Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
        {{}, {{filterLabel("Platinum Ore Piece"), 4, {0, 1, 3, 4}}}, 16},
      }));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "center", "5cb", Actions::up, Actions::south,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 64)));

    // manufactory
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "manufactory", "center", "5cb", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 27, nullptr, std::vector<Recipe<>>{
        {{{filterLabel("Sand"), 64}}, {{filterLabel("Cobblestone"), 1}}},
        {{{filterLabel("Niter"), 64}}, {{filterLabel("Sandstone"), 1}}}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
