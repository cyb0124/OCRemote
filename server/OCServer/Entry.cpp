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
    factory.addChest({2813, 143, -1267});
    factory.addChest({2813, 143, -1272});
    factory.addChest({2813, 144, -1272});
    factory.addChest({2813, 145, -1272});
    factory.addChest({2813, 146, -1272});
    factory.addChest({2813, 147, -1272});
    factory.addChest({2811, 144, -1272});
    factory.addChest({2811, 145, -1272});
    factory.addChest({2811, 146, -1272});
    factory.addChest({2811, 147, -1272});
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "furnace", XNetCoord{2811, 143, -1270}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Charcoal"), 256}}, {{filterLabel("Oak Wood"), 1}}},
        {{{filterLabel("Iron Ingot"), 256}}, {{filterLabel("Pulverized Iron"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "enrichment", XNetCoord{2813, 143, -1269}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Pulverized Iron"), 16}}, {{filterName("sky_orchards:amber_iron"), 1}}},
        {{{filterLabel("Iron Plate"), 16}}, {{filterLabel("Iron Ingot"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "crusher", XNetCoord{2812, 143, -1266}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{{filterLabel("Bio Fuel"), 32}}, {{filterLabel("Beetroot"), 1}}}
      }));
    factory.addProcess(std::make_shared<ProcessSingleBlock>(
      "PRC", XNetCoord{2810, 143, -1266}, Actions::top, 16, true, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Bio Fuel"), 1}}}
      }));
    factory.addBackup(filterLabel("Beetroot Seeds"), 8);
    factory.addBackup(filterLabel("Menril Sapling"), 8);
    factory.addBackup(filterLabel("Menril Leaves"), 8);
    std::vector<Recipe<int>> recipesPlanter{
      {{{filterLabel("Beetroot Seeds"), 32}, {filterLabel("Beetroot"), 32}}, {{filterLabel("Beetroot Seeds"), 1, true}}},
      {{{filterLabel("Menril Leaves"), 16}, {filterLabel("Menril Wood"), 32}}, {{filterLabel("Menril Sapling"), 1, true}}}
    }, recipesLeafBreaker{
      {{{filterLabel("Menril Sapling"), 32}, {filterLabel("Menril Berries"), 32}}, {{filterLabel("Menril Leaves"), 1, true}}}
    };
    auto addOreTree([&](const std::string &name, std::optional<XNetCoord> amberCrafter) {
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
      if (amberCrafter) {
        factory.addProcess(std::make_shared<ProcessSingleBlock>(
          name + "Amber", *amberCrafter, Actions::top, 144, false, std::vector<Recipe<int>>{
            {{{amber, 16}}, {{acorn, 4}, {resin, 5}}}
          }));
      }
    });
    addOreTree("iron"      , XNetCoord{2811, 143, -1268});
    addOreTree("copper"    , std::nullopt);
    addOreTree("tin"       , std::nullopt);
    addOreTree("lead"      , std::nullopt);
    addOreTree("osmium"    , std::nullopt);
    addOreTree("silver"    , std::nullopt);
    addOreTree("coal"      , std::nullopt);
    addOreTree("nickel"    , std::nullopt);
    addOreTree("redstone"  , std::nullopt);
    addOreTree("gold"      , std::nullopt);
    addOreTree("cottonwood", std::nullopt);
    addOreTree("bone"      , std::nullopt);
    addOreTree("sand"      , std::nullopt);
    addOreTree("dirt"      , std::nullopt);
    addOreTree("clay"      , std::nullopt);
    addOreTree("petrified" , std::nullopt);
    addOreTree("gravel"    , std::nullopt);
    addOreTree("lapis"     , std::nullopt);
    addOreTree("diamond"   , std::nullopt);
    addOreTree("emerald"   , std::nullopt);
    addOreTree("netherrack", std::nullopt);
    addOreTree("glowstone" , std::nullopt);
    addOreTree("quartz"    , std::nullopt);
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
