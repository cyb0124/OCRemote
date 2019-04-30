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
    Factory factory(server, "shadownode-sf4-base", "xnet", "me_interface", {2811, 143, -1272}, {2810, 143, -1273}, 1000);
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "furnace", XNetCoord{2813, 143, -1271}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Charcoal"), 256}}, {{filterLabel("Oak Wood"), 1}}},
        {{{filterLabel("Iron Ingot"), 256}}, {{filterLabel("Pulverized Iron"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "enrichment", XNetCoord{2813, 143, -1270}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Pulverized Iron"), 16}}, {{filterName("sky_orchards:amber_iron"), 1}}},
        {{{filterLabel("Iron Plate"), 16}}, {{filterLabel("Iron Ingot"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "crusher", XNetCoord{2811, 143, -1271}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Bio Fuel"), 32}}, {{filterLabel("Beetroot"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "PRC", XNetCoord{2813, 143, -1268}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Bio Fuel"), 1}}}
      }));
    factory.addBackup(filterLabel("Seeds"), 8);
    factory.addBackup(filterLabel("Beetroot Seeds"), 8);
    factory.addBackup(filterLabel("Menril Sapling"), 8);
    factory.addBackup(filterLabel("Menril Leaves"), 8);
    std::vector<Recipe<int>> recipesPlanter{
      {{{filterLabel("Seeds"), 32}, {filterLabel("Wheat"), 32}}, {{filterLabel("Seeds"), 1, true}}},
      {{{filterLabel("Beetroot Seeds"), 32}, {filterLabel("Beetroot"), 32}}, {{filterLabel("Beetroot Seeds"), 1, true}}},
      {{{filterLabel("Menril Leaves"), 16}, {filterLabel("Menril Wood"), 32}}, {{filterLabel("Menril Sapling"), 1, true}}}
    }, recipesLeafBreaker{
      {{{filterLabel("Menril Sapling"), 32}, {filterLabel("Menril Berries"), 32}}, {{filterLabel("Menril Leaves"), 1, true}}}
    };
    auto addOreTree([&](const std::string &name) {
      auto sapling(filterName("sky_orchards:sapling_" + name));
      auto leaves(filterName("sky_orchards:leaves_" + name));
      auto resin(filterName("sky_orchards:resin_" + name));
      auto acorn(filterName("sky_orchards:acorn_" + name));
      auto amber(filterName("sky_orchards:amber_" + name));
      factory.addBackup(sapling, 8);
      factory.addBackup(leaves, 8);
      recipesPlanter.push_back({
        {{filterLabel("Oak Wood"), 256}, {leaves, 16}, {resin, 32}},
        {{sapling, 1, true}}
      });
      recipesLeafBreaker.push_back({
        {{acorn, 32}, {sapling, 32}},
        {{leaves, 1, true}}
      });
    });
    addOreTree("iron"      );
    addOreTree("copper"    );
    addOreTree("tin"       );
    addOreTree("lead"      );
    addOreTree("osmium"    );
    addOreTree("silver"    );
    addOreTree("coal"      );
    addOreTree("nickel"    );
    addOreTree("redstone"  );
    addOreTree("gold"      );
    addOreTree("cottonwood");
    addOreTree("bone"      );
    addOreTree("sand"      );
    addOreTree("dirt"      );
    addOreTree("clay"      );
    addOreTree("petrified" );
    addOreTree("gravel"    );
    addOreTree("lapis"     );
    addOreTree("diamond"   );
    addOreTree("emerald"   );
    addOreTree("netherrack");
    addOreTree("glowstone" );
    addOreTree("quartz"    );
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "planter", XNetCoord{2808, 149, -1272}, Actions::top, 16, false, std::move(recipesPlanter)));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "leafBreaker", XNetCoord{2806, 148, -1272}, Actions::top, 16, false, std::move(recipesLeafBreaker)));
    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
