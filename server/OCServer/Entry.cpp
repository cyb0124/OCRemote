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
    Factory factory(server, 1000, "center", {{"center", "5cb", Actions::south}});
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "5cb", Actions::east, Actions::south));
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "a12", Actions::east, Actions::north));
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "3c3", Actions::south, Actions::west));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "6ef", Actions::up, Actions::west, std::vector<SharedItemFilter>{
      filterLabel("Cobblestone"), filterLabel("Sand"),
      filterLabel(u8"§eInferium Essence"),
      filterLabel("Platinum Ore Chunk"),
      filterLabel("Silver Ore Chunk"),
      filterLabel("Iron Ore Chunk"),
      filterLabel("Gold Ore Chunk"),
      filterLabel("Tin Ore Chunk")
    }));

    // reactor
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "center"));

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "5cb", Actions::north, Actions::south,
      std::vector<StockEntry>{}, 0, outAll, std::vector<Recipe<int>>{}));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "center", "f80", "a12",
      Actions::east, Actions::north, -1, std::vector<Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Netherrack"), 64}}, {{filterLabel("Nether Essence"), 5, {2, 4, 5, 6, 8}}}, {2, std::nullopt}},
        {{{filterLabel("Soul Sand"), 64}}, {{filterLabel("Nether Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {3, std::nullopt}},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Enderium Ingot"), 64}}, {{filterLabel("Enderium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Lapis Lazuli"), 64}}, {{filterLabel("Lapis Lazuli Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {5, std::nullopt}},
        {{{filterLabel("Glowstone Dust"), 64}}, {{filterLabel("Glowstone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Certus Quartz Crystal"), 64}}, {{filterLabel("Certus Quartz Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Redstone"), 64}}, {{filterLabel("Redstone Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Diamond Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Compressed Netherrack"), 16}}, {{filterLabel("Netherrack"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 5, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Oak Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 2, 3}}}, {4, std::nullopt}},
        {{{filterLabel("Sandstone"), 16}}, {{filterLabel("Sand"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Bronze Gear"), 16}}, {{filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Silver Gear"), 16}}, {{filterLabel("Silver Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Weighted Pressure Plate (Heavy)"), 16}}, {{filterLabel("Iron Ingot"), 2, {1, 2}}}, {16, std::nullopt}},
        {{{filterLabel("Weighted Pressure Plate (Light)"), 16}}, {{filterLabel("Gold Ingot"), 2, {1, 2}}}, {16, std::nullopt}},
        {{{filterLabel("Rich Phyto-Gro"), 16}}, {{filterLabel("Pulverized Charcoal"), 1, {1}},
          {filterLabel("Niter"), 1, {2}}, {filterLabel("Rich Slag"), 1, {3}}}, {4, std::nullopt}},
        {{{filterLabel("Clock"), 16}}, {
          {filterLabel("Redstone"), 1, {5}},
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Redstone Reception Coil"), 16}}, {
          {filterLabel("Redstone"), 2, {3, 7}},
          {filterLabel("Gold Ingot"), 1, {5}}
        }, {16, std::nullopt}},
        {{{filterLabel("Redstone Servo"), 16}}, {
          {filterLabel("Redstone"), 2, {2, 8}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {16, std::nullopt}},
        {{{filterLabel("Augment: Auxiliary Reception Coil"), 4}}, {
          {filterLabel("Redstone Reception Coil"), 1, {5}},
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}}
        }, {4, std::nullopt}},
        {{{filterLabel("Augment: Auxiliary Sieve"), 4}}, {
          {filterLabel("Redstone Servo"), 1, {5}},
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}
        }, {4, std::nullopt}},
        {{{filterLabel("Hardened Upgrade Kit"), 4}}, {
          {filterLabel("Bronze Gear"), 1, {5}},
          {filterLabel("Redstone"), 2, {7, 9}},
          {filterLabel("Invar Ingot"), 4, {2, 4, 6, 8}}
        }, {4, std::nullopt}},
        {{{filterLabel("Reinforced Upgrade Kit"), 4}}, {
          {filterLabel("Silver Gear"), 1, {5}},
          {filterLabel("Hardened Glass"), 2, {7, 9}},
          {filterLabel("Electrum Ingot"), 4, {2, 4, 6, 8}}
        }, {4, std::nullopt}},
        {{{filterLabelName("Speed Upgrade", "nuclearcraft:upgrade"), 64}}, {
          {filterLabel("Weighted Pressure Plate (Heavy)"), 1, {5}},
          {filterLabel("Redstone"), 4, {2, 4, 6, 8}},
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}}
        }, {16, std::nullopt}},
        {{{filterLabelName("Energy Upgrade", "nuclearcraft:upgrade"), 64}}, {
          {filterLabel("Weighted Pressure Plate (Light)"), 1, {5}},
          {filterLabel("Crushed Quartz"), 4, {2, 4, 6, 8}},
          {filterLabel("Pulverized Obsidian"), 4, {1, 3, 7, 9}}
        }, {16, std::nullopt}},
        {{}, {{filterLabel("Yellorium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Titanium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Tungsten Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Platinum Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Iridium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Mithril Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Copper Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Ardite Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Cobalt Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Osmium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Silver Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Nickel Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Lead Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Gold Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Iron Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Zinc Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{}, {{filterLabel("Tin Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}}
      }));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "cobbleGen", "center", "6ef", Actions::south, Actions::west,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 640)));

    // snowballGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "snowballGen", "center", "5cb", Actions::up, Actions::south,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Snowball"), 64)));

    // obsGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "obsGen", "center", "f80", Actions::south, Actions::east,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Obsidian"), 64)));

    // manufactory
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "manufactory", "center", "5cb", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 32, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Sand"), 640}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Niter"), 16}}, {{filterLabel("Sandstone"), 1}}, INT_MAX},
        {{{filterLabel("Silicon"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Charcoal"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Obsidian"), 16}}, {{filterLabel("Obsidian"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Lead"), 16}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Dust"), 16}}, {{filterLabel("Graphite Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Crushed Quartz"), 16}}, {{filterLabel("Nether Quartz"), 1}}, INT_MAX}
      }));

    // sagMill
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sagMill", "center", "c28", Actions::north, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Crushed Netherrack"), 64}}, {{filterLabel("Compressed Netherrack"), 1}}, INT_MAX}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "furnace", "center", "a12", Actions::west, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Glass"), 64}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Stone"), 64}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Oak Wood"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Ingot"), 64}}, {{filterLabel("Charcoal"), 1}}, INT_MAX}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "a12", Actions::south, Actions::north,
      std::vector<StockEntry>{
        {filterLabel("Fluxed Phyto-Gro"), 16},
        {filterLabel("Yellorium Ingot"), 16},
        {filterLabel("Cobblestone"), 16}
      }, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§eInferium Essence"), 4096}}, {{filterName("mysticalagriculture:tier5_inferium_seeds"), 1}}, INT_MAX},
        {{{filterLabel("Certus Quartz Essence"), 16}}, {{filterLabel("Certus Quartz Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Essence"), 16}}, {{filterLabel("Lapis Lazuli Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Essence"), 16}}, {{filterLabel("Glowstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 16}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Enderium Essence"), 16}}, {{filterLabel("Enderium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Essence"), 16}}, {{filterLabel("Diamond Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Essence"), 16}}, {{filterLabel("Cobalt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nether Essence"), 16}}, {{filterLabel("Nether Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Wood Essence"), 16}}, {{filterLabel("Wood Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Ender Pearl"), 64}}, {{filterLabel("Ender Lilly"), 1}}, INT_MAX},
        {{{filterLabel("Seeds"), 16}}, {{filterLabel("Seeds"), 1}}, INT_MAX}
      }));

    // sandInduction
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sandInduction", "center", "f80", Actions::west, Actions::east,
      std::vector<StockEntry>{{filterLabel("Sand"), 16}}, 32, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Rich Slag"), 16}}, {{filterLabel("Clock"), 1}}, 16},
        {{{filterLabel("Tin Ingot"), 64}}, {{filterLabel("Tin Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Iron Ingot"), 64}}, {{filterLabel("Iron Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Gold Ingot"), 64}}, {{filterLabel("Gold Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Lead Ingot"), 64}}, {{filterLabel("Lead Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Zinc Ingot"), 64}}, {{filterLabel("Zinc Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Copper Ingot"), 64}}, {{filterLabel("Copper Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Ingot"), 64}}, {{filterLabel("Osmium Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Silver Ingot"), 64}}, {{filterLabel("Silver Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Nickel Ingot"), 64}}, {{filterLabel("Nickel Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Platinum Ingot"), 64}}, {{filterLabel("Platinum Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Yellorium Ingot"), 64}}, {{filterLabel("Yellorium Ore Chunk"), 1}}, INT_MAX}
      }));

    // sieve
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sieve", "center", "6ef", Actions::east, Actions::west,
      std::vector<StockEntry>{}, 32, nullptr, std::vector<Recipe<int>>{
        {{
          {filterLabel("Tin Ore Chunk"), 16},
          {filterLabel("Iron Ore Chunk"), 16},
          {filterLabel("Gold Ore Chunk"), 16},
          {filterLabel("Lead Ore Chunk"), 16},
          {filterLabel("Zinc Ore Chunk"), 16},
          {filterLabel("Copper Ore Chunk"), 16},
          {filterLabel("Osmium Ore Chunk"), 16},
          {filterLabel("Silver Ore Chunk"), 16},
          {filterLabel("Nickel Ore Chunk"), 16},
          {filterLabel("Platinum Ore Chunk"), 16},
          {filterLabel("Yellorium Ore Chunk"), 16}
        }, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Prosperity Shard"), 64}}, {{filterLabel("Crushed Netherrack"), 1}}, INT_MAX},
        {{{filterLabel("Nether Quartz"), 64}}, {{filterLabel("Soul Sand"), 1}}, INT_MAX}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "f80", Actions::up, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "center", "6ef", Actions::down, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Steel Ingot"), 16}}, {{filterLabel("Iron Ingot"), 1, {0}}, {filterLabel("Graphite Ingot"), 1, {1}}}, 16},
        {{{filterLabel("Bronze Ingot"), 16}}, {{filterLabel("Copper Ingot"), 3, {0}}, {filterLabel("Tin Ingot"), 1, {1}}}, 16},
        {{{filterLabel("Invar Ingot"), 16}}, {{filterLabel("Iron Ingot"), 2, {0}}, {filterLabel("Nickel Ingot"), 1, {1}}}, 16},
        {{{filterLabel("Electrum Ingot"), 16}}, {{filterLabel("Gold Ingot"), 1, {0}}, {filterLabel("Silver Ingot"), 1, {1}}}, 16}
      }));

    // inductionSmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "inductionSmelter", "center", "3c3", Actions::north, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Hardened Glass"), 16}}, {{filterLabel("Pulverized Lead"), 1, {0}}, {filterLabel("Pulverized Obsidian"), 4, {1}}}, 16}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
