#include <stdexcept>
#include <iostream>
#include <csignal>
#include "Server.h"
#include "Factory.h"
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
    Factory factory(server, "shadownode-sf4-base", "xnet", {2811, 143, -1272}, 1000);
    factory.addChest({2813, 143, -1272});
    factory.addBackup(filterName("sky_orchards:sapling_iron"), 8);
    factory.addProcess(std::make_shared<ProcessHeterogeneous>(
      "planter", XNetCoord{2808, 149, -1272}, Actions::top, 16, false, std::vector<Recipe<std::monostate>>{
        {{{filterName("sky_orchards:sapling_iron"), 32},
          {filterLabel("Oak Wood"), 256},
          {filterName("sky_orchards:leaves_iron"), 32},
          {filterName("sky_orchards:resin_iron"), 32}
          }, {{filterName("sky_orchards:sapling_iron"), 1, true}}}
      }
    ));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
