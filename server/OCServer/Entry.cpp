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
    factory.addChest({2813, 144, -1272});
    factory.addChest({2811, 144, -1272});
    std::vector<Recipe<std::monostate>> recipesPlanter, recipesLeafBreaker;
    auto addOreTree([&](const std::string &name) {
      auto sapling(filterName("sky_orchards:sapling_" + name));
      auto leaves(filterName("sky_orchards:leaves_" + name));
      auto resin(filterName("sky_orchards:resin_" + name));
      auto acorn(filterName("sky_orchards:acorn_" + name));
      factory.addBackup(sapling, 8);
      factory.addBackup(leaves, 8);
      recipesPlanter.push_back({
        {{filterLabel("Oak Wood"), 256}, {leaves, 16}, {resin, 64}},
        {{sapling, 1, true}}
      });
      recipesLeafBreaker.push_back({
        {{acorn, 64}, {sapling, 16}},
        {{leaves, 1, true}}
      });
    });
    addOreTree("iron");
    addOreTree("copper");
    addOreTree("tin");
    addOreTree("lead");
    addOreTree("osmium");
    addOreTree("silver");
    addOreTree("coal");
    addOreTree("nickel");
    addOreTree("redstone");
    addOreTree("gold");
    addOreTree("cottonwood");
    addOreTree("bone");
    addOreTree("sand");
    addOreTree("dirt");
    addOreTree("clay");
    addOreTree("petrified");
    addOreTree("gravel");
    addOreTree("lapis");
    addOreTree("diamond");
    addOreTree("emerald");
    addOreTree("netherrack");
    addOreTree("glowstone");
    addOreTree("quartz");
    factory.addProcess(std::make_shared<ProcessHeterogeneous>(
      "planter", XNetCoord{2808, 149, -1272}, Actions::top, 16, false, std::move(recipesPlanter)));
    factory.addProcess(std::make_shared<ProcessHeterogeneous>(
      "leafBreaker", XNetCoord{2806, 148, -1272}, Actions::top, 16, false, std::move(recipesLeafBreaker)));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
