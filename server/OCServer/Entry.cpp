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
    Factory factory(server, 1000, "center", "e69", Actions::up);
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "e69", Actions::east, Actions::up));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "e69", Actions::down, Actions::up, std::vector<SharedItemFilter>{
      filterLabel("Cobblestone"), filterLabel("Flint"), filterLabel("Lapis Lazuli"), filterLabel("Sky Stone Dust"),
      filterLabel("item.projectred.core.itemResource.electrotine_dust.name"),
      filterLabel("Crushed Quartz"), filterLabel("Certus Quartz Dust"), filterLabel("Bone Meal"), filterLabel("Coal"),
      filterLabel("Birch Sapling"), filterLabel("Birch Wood"),
      filterLabel("Oak Sapling"), filterLabel("Oak Wood"), filterLabel("Apple"),
      filterLabel("Iron Ore Piece"), filterLabel("Gold Ore Piece"), filterLabel("Aluminum Ore Piece"), filterLabel("Nickel Ore Piece"),
      filterLabel("Silver Ore Piece"), filterLabel("Osmium Ore Piece"), filterLabel("Lead Ore Piece"), filterLabel("Tin Ore Piece"),
      filterLabel("Copper Ore Piece"), filterLabel("Platinum Ore Piece"), filterLabel("Yellorium Ore Piece"), filterLabel("Zinc Ore Piece")
    }));
    // factory.addProcess(std::make_unique<ProcessInputless>(factory, "center", "fc3", Actions::north, Actions::south,
    //  0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 1024)));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorOut", "reactor", "010", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessHeterogeneousWorkingSet>(factory, "reactorIn", "reactor", "010", Actions::north, Actions::up,
      std::vector<StockEntry>{{filterLabel("Yellorium Ingot"), 8}}, 0, [](auto&&...) { return true; }, std::vector<Recipe<>>{}));
    factory.addProcess(std::make_unique<ProcessReactorHysteresis>(factory, "reactor", "reactor"));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
