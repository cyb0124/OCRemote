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
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "center", "377", Actions::south, Actions::west,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 32)));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pulverizer", "center", "377", Actions::north, Actions::west, std::vector<size_t>{0},
      [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gravel"), 32}}, {{filterLabel("Cobblestone"), 1, false, std::vector<size_t>{0}}}, 8},
        {{{filterLabel("Sand"), 32}}, {{filterLabel("Gravel"), 1, false, std::vector<size_t>{0}}}, 8}
      }));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manufactory", "center", "377", Actions::up, Actions::west, std::vector<size_t>{0},
      [](size_t slot, auto&&) { return 1 == slot; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Niter"), 32}}, {{filterLabel("Sandstone"), 1, false, std::vector<size_t>{0}}}, 8}
      }));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorOut", "reactor", "010", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorIn", "reactor", "010", Actions::north, Actions::up,
      std::vector<StockEntry>{{filterLabel("Yellorium Ingot"), 8}}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessReactorHysteresis>(factory, "reactor", "reactor"));
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "analogCrafter", "reactor", "010", Actions::east, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8}, [](auto&&...) { return true; }, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sandstone"), 32}}, {{filterLabel("Sand"), 4, false, std::vector<size_t>{0, 1, 3, 4}}}, 8}
      }));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
