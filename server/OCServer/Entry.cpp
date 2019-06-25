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
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "e69", Actions::down, Actions::up));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "e69", Actions::east, Actions::up, std::vector<SharedItemFilter>{
      filterLabel("Cobblestone"),
      filterLabel("Birch Sapling"), filterLabel("Birch Wood"),
      filterLabel("Oak Sapling"), filterLabel("Oak Wood"), filterLabel("Apple"),
      filterLabel("Iron Ore Piece")
    }));
    factory.addBackup(filterLabel("Oak Sapling"), 8);
    factory.addBackup(filterLabel("Birch Sapling"), 8);
    factory.addProcess(std::make_unique<ProcessWorkingSet>(factory, "center", "bb0", Actions::up, Actions::south,
      [](auto&&...) { return true; }, std::vector<Recipe<std::pair<std::string, int>>>{}));
    factory.addProcess(std::make_unique<ProcessScatteringWorkingSet>(factory, "plantSower", "center", "fc3",
      Actions::up, Actions::south, 8, plantSowerInSlots(), nullptr, std::vector<Recipe<>>{
        {{{filterLabel("Oak Sapling"), 256}, {filterLabel("Oak Wood"), 1024}, {filterLabel("Apple"), 256}},
          {{filterLabel("Oak Sapling"), 1, true}}},
        {{{filterLabel("Birch Sapling"), 256}, {filterLabel("Birch Wood"), 1024}},
          {{filterLabel("Birch Sapling"), 1, true}}}
      }));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
