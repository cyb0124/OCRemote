#include <stdexcept>
#include <iostream>
#include <csignal>
#include <boost/algorithm/string.hpp>
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
    Factory factory(server, 1000, "north", {{"center", "441", Actions::east}});
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "441", Actions::west, Actions::east, std::vector<SharedItemFilter>{
      filterLabel("Cobblestone"), filterLabel("Gravel"), filterLabel("Flint"), filterLabel("Gunpowder"),
      filterLabel("Blaze Powder"), filterLabel("Redstone"), filterLabel("Glowstone Dust"), filterLabel("Nether Quartz"),
      filterLabel("Lapis Lazuli"), filterLabel("Certus Quartz Crystal"), filterLabel("Diamond"), filterLabel("Emerald"),
      filterLabel("Birch Wood Planks"), filterLabel("Stick"), filterLabel("Sawdust"), filterLabel("Dirty Quartz"),
      filterLabel("Sand"), filterLabel("Glass"), filterLabel("Stone"), filterLabel("Netherrack"),
      filterLabel("Potato"), filterLabel("Poisonous Potato"), filterLabel("Bio Fuel"), filterLabel("Carrot"),
      filterLabel("Birch Wood"), filterLabel("Birch Sapling"), filterLabel("Charcoal"), filterLabel("Coal"),
      filterLabel("Crushed Stone"), filterLabel("Dirty Emerald"), filterLabel("Dirty Diamond"), filterLabel("Dirty Ruby"),
      filterLabel("Dirty Sapphire"), filterLabel("Dirty Peridot"), filterLabel("Dirty Apatite"), filterLabel("Dirty Amber"),
      filterLabel("Dirty Aquamarine"), filterLabel("Dirty Lapis Lazuli"), filterLabel("Dirty Black Quartz"), filterLabel("Dirty Certus Quartz"),
      filterLabel("Dirty Charged Certus Quartz"), filterLabel("Dirty Coal"), filterLabel("Crushed Netherrack"), filterLabel("Dirty Malachite"),
      filterLabel("Wheat"), filterLabel("Seeds"), filterLabel("Tertius Alchemical Dust"), filterLabel("Crystal Shard"),
      filterLabel("Gold Ingot"), filterLabel("Alchemical Gold Ingot"), filterLabel("Gold Alchemical Ore Dust"), filterLabel("Osmium Dust"),
      filterLabel("Osmium Ingot"), filterLabel("Osmium Alchemical Ore Dust"), filterLabel("Iron Alchemical Ore Dust"), filterLabel("Aluminum Ingot"),
      filterLabel("Cobalt Ingot"), filterLabel("Cobalt Alchemical Ore Dust"), filterLabel("Cobalt Ore Dust"), filterLabel("Ardite Ore Dust"),
      filterLabel("Ardite Alchemical Ore Dust"), filterLabel("Ardite Ingot"), filterLabel("Pulverized Iron"), filterLabel("Iron Ingot"),
      filterLabel("Aluminum Alchemical Ore Dust"), filterLabel("Pulverized Aluminum"), filterLabel("Nickel Ingot"), filterLabel("Nickel Alchemical Ore Dust"),
      filterLabel("Silver Ingot"), filterLabel("Pulverized Silver"), filterLabel("Copper Ingot"), filterLabel("Pulverized Lead"),
      filterLabel("Silver Alchemical Ore Dust"), filterLabel("Pulverized Copper"), filterLabel("Copper Alchemical Ore Dust"), filterLabel("Lead Ingot"),
      filterLabel("Tin Ingot"), filterLabel("Lead Alchemical Ore Dust"), filterLabel("Pulverized Tin"), filterLabel("Tin Alchemical Ore Dust"),
      filterLabel("Aquamarine"), filterLabel("Plant Matter"), filterLabel("Graphite Ingot"), filterLabel("Rice Seeds"),
      filterLabel("String"), filterLabel(u8"§eInferium Essence"), filterLabel("Rotten Flesh"), filterLabel("Bone"),
      filterLabel("Arrow"), filterLabel("Spider Eye"), filterLabel("Zombie Brain"), filterLabel("Ender Pearl"),
      filterLabel("Bronze Ingot"), filterLabel("Steel Ingot"), filterLabel("Invar Ingot"), filterLabel("Silicon Ingot"),
      filterLabel("Electrical Steel Ingot"), filterLabel("Electrum Ingot"), filterLabel("Tiny Dry Rubber"), filterLabel("Dry Rubber"),
      filterLabel("TNT"), filterLabel("Agave"), filterLabel("Soybean"), filterLabel("Sugar"),
      filterLabel("Silken Tofu"), filterLabel("Grain Bait"), filterLabel("Firm Tofu"), filterLabel("Soy Milk"),
      filterLabel("Plastic"), filterLabel("Dirt"), filterLabel("Stardust"), filterLabel("Bone Meal"),
      filterLabel("Radioactive Powder Mixture"), filterLabel("Uranium Alchemical Ore Dust"), filterLabel("Uranium Ingot"), filterLabel("Impregnated Stick"),
      filterLabel("Iron Plate"), filterLabel("Block of Iron"), filterLabel("Iron Large Plate"), filterLabel("Aluminum Plate"),
      filterLabel("Rice"), filterLabel("Paper"), filterLabel("Rice Dough"), filterLabel("Basic Machine Casing"),
      filterLabel("Bronze Plate"), filterLabel("Tin Plate"), filterLabel("Steel Plate"), filterLabel("Mixed Metal Ingot"),
      filterLabel("Advanced Alloy"), filterLabel("Pulverized Coal"), filterLabel("Raw Carbon Fibre"), filterLabel("Raw Carbon Mesh"),
      filterLabel("Carbon Plate"), filterLabel("Advanced Machine Casing"), filterLabel("Compressed Sand"), filterLabel("Dust"),
      filterLabel("Uranium Plate"), filterName("minecraft:clay"), filterName("minecraft:clay_ball"), filterLabel("Clay Dust"),
      filterLabel("Reinforced Stone"), filterLabel("Grout"), filterLabel("Machine Case"), filterLabel("Rice Slimeball"),
      filterLabel("Slime Block"), filterLabel("Slimeball"), filterLabel("Blaze Powder Block"), filterLabel("Obsidian"),
      filterLabel("Enori Crystal"), filterLabel("Copper Gear"), filterLabel("Pink Slime"), filterLabel("Restonia Crystal"),
      filterLabel("Gold Cable"), filterLabel("Aluminium Wire"), filterLabel("Basic Coil"), filterLabel("Advanced Coil"),
      filterLabel("Device Frame"), filterLabel("Copper Cable"), filterLabel("Iron Sheetmetal"), filterLabel("Gold Gear"),
      filterLabel("Glass Pane"), filterLabel("Steel Sheetmetal"), filterLabel("Iron Mechanical Component"), filterLabel("Coil"),
      filterLabel("Sturdy Casing"), filterLabel("Bronze Gear"), filterLabel("Steel Rod"), filterLabel("Compressed Redstone"),
      filterLabel("Compressed Diamond"), filterLabel("Enriched Alloy"), filterLabel("Basic Control Circuit"),
      filterLabel("Tin Item Casing")
    }));
    factory.addStorage(std::make_unique<StorageChest>(factory, "north", "334", Actions::up, Actions::east));
    factory.addBackup(filterLabel("Potato"), 32);
    factory.addBackup(filterLabel("Agave"), 32);
    factory.addBackup(filterLabel("Soybean"), 32);
    factory.addBackup(filterLabel("Seeds"), 32);

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "f98", Actions::south, Actions::west,
      std::vector<StockEntry>{}, INT_MAX, outAll, std::vector<Recipe<int>>{}));

    // stock
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "stock", "center", "f98", Actions::up, Actions::west,
      std::vector<StockEntry>{
        {filterLabel("Crystal Shard"), 16},
        {filterLabel("Bio Fuel"), 16},
        {filterLabel("Tertius Alchemical Dust"), 16},
        {filterLabel("TNT"), 16},
        {filterLabel("Seeds"), 16},
        {filterLabel("Birch Wood"), 16},
        {filterLabel("Blaze Powder Block"), 16},
        {filterLabel("Compressed Redstone"), 16}
      }, INT_MAX, nullptr, std::vector<Recipe<int>>{}));

    // trash
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "trash", "north", "334", Actions::north, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Bow"), 1}}, INT_MAX},
        {{}, {{filterLabel("Witch Hat"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Shovel"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Sword"), 1}}, INT_MAX},
        {{}, {{filterLabel("Leather Cap"), 1}}, INT_MAX},
        {{}, {{filterLabel("Leather Tunic"), 1}}, INT_MAX},
        {{}, {{filterLabel("Leather Pants"), 1}}, INT_MAX},
        {{}, {{filterLabel("Leather Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Iron Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Boots"), 1}}, INT_MAX}
      }));

    // platePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "platePress", "north", "0b8", Actions::north, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Iron Plate"), 64}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bronze Plate"), 64}}, {{filterLabel("Bronze Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Tin Plate"), 64}}, {{filterLabel("Tin Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Steel Plate"), 64}}, {{filterLabel("Steel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Plate"), 64}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Plate"), 64}}, {{filterLabel("Uranium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Large Plate"), 4}}, {{filterLabel("Block of Iron"), 1}}, INT_MAX}
      }));

    // rodPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "rodPress", "center", "12a", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Steel Rod"), 4}}, {{filterLabel("Steel Sheetmetal"), 1}}, INT_MAX}
      }));

    // gearPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "gearPress", "north", "1e4", Actions::north, Actions::west,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Copper Gear"), 16}}, {{filterLabel("Copper Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Bronze Gear"), 16}}, {{filterLabel("Bronze Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Gold Gear"), 16}}, {{filterLabel("Gold Ingot"), 4}}, INT_MAX}
      }));

    // wirePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "wirePress", "center", "5a0", Actions::up, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Wire"), 64}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX}
      }));

    // pinkSlime
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pinkSlime", "center", "12a", Actions::up, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Pink Slime"), 64}}, {{filterLabel("Slime Block"), 1}}, INT_MAX}
      }));

    // atomic
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "atomic", "center", "5a0", Actions::south, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Enori Crystal"), 64}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Restonia Crystal"), 64}}, {{filterLabel("Redstone"), 1}}, INT_MAX}
      }));

    // clayBarrel
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "clayBarrel", "north", "1e4", Actions::south, Actions::west,
      std::vector<StockEntry>{}, INT_MAX, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:clay"), 64}}, {{filterLabel("Dust"), 1}}, 16}
      }));

    // combustionA
    factory.addProcess(std::make_unique<ProcessRedstoneConditional>(factory, "combustionA", "center", "ed8", Actions::north, true,
      [&](int value) { return value > 0; }, std::make_unique<ProcessSlotted>(factory, "combustionA", "center", "f98", Actions::east, Actions::west,
      std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Redstone"), 256}}, {
          {filterLabel("Blaze Powder"), 2, {0}},
          {filterLabel("Gunpowder"), 2, {1}}
        }, 16},
        {{{filterLabel("Glowstone Dust"), 256}}, {
          {filterLabel("Blaze Powder"), 2, {0}},
          {filterLabel("Redstone"), 4, {1}}
        }, 16},
        {{{filterLabel("Netherrack"), 64}}, {
          {filterLabel("Blaze Powder"), 3, {0}},
          {filterLabel("Cobblestone"), 8, {1}}
        }, 16},
        {{{filterLabel("Radioactive Powder Mixture"), 16}}, {
          {filterLabel("Gunpowder"), 4, {0}},
          {filterLabel("Spider Eye"), 2, {1}},
          {filterLabel("Poisonous Potato"), 1, {2}}
        }, 16}
      })));

    // combustionB
    factory.addProcess(std::make_unique<ProcessRedstoneConditional>(factory, "combustionB", "center", "ed8", Actions::south, true,
      [&](int value) { return value > 0; }, std::make_unique<ProcessSlotted>(factory, "combustionB", "center", "d40", Actions::south, Actions::up,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gunpowder"), 256}}, {
          {filterLabel("Flint"), 1, {0}}
        }, 16},
        {{{filterLabel("Blaze Powder"), 256}}, {
          {filterLabel("Gunpowder"), 1, {0}}
        }, 16},
        {{{filterLabel("Coal"), 64}}, {
          {filterLabel("Charcoal"), 1, {0}}
        }, 16},
        {{{filterLabel("Poisonous Potato"), 64}}, {
          {filterLabel("Potato"), 4, {0}},
          {filterLabel("Rotten Flesh"), 1, {1}}
        }, 16}
      })));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "cobbleGen", "center", "258", Actions::east, Actions::down,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 256)));

    // obsidianGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "obsidianGen", "center", "5a0", Actions::west, Actions::north,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Obsidian"), 64)));

    // rubberGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "rubberGen", "north", "0b8", Actions::south, Actions::east,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Tiny Dry Rubber"), 64)));

    // planter
    factory.addProcess(std::make_unique<ProcessScatteringWorkingSet>(factory, "planter", "center", "258", Actions::up, Actions::down,
      4, std::vector<size_t>{6, 7, 8, 9, 11, 12, 13, 14}, nullptr, std::vector<Recipe<>>{
        {{{filterLabel("Potato"), 64}}, {{filterLabel("Potato"), 1, {}, true}}},
        {{{filterLabel("Carrot"), 64}}, {{filterLabel("Carrot"), 1}}},
        {{{filterLabel("Agave"), 64}}, {{filterLabel("Agave"), 1, {}, true}}},
        {{{filterLabel("Soybean"), 64}}, {{filterLabel("Soybean"), 1, {}, true}}},
        {{{filterLabel("Rice"), 64}}, {{filterLabel("Rice Seeds"), 1}}},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Birch Sapling"), 1}}},
        {{{filterLabel("Seeds"), 64}, {filterLabel("Wheat"), 64}}, {{filterLabel("Seeds"), 1, {}, true}}}
      }));

    // alchemicalA
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alchemicalA", "center", "f98", Actions::north, Actions::west,
      std::vector<size_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gold Alchemical Ore Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Wheat"), 1, {2}}
        }, 8},
        {{{filterLabel("Alchemical Gold Ingot"), 16}}, {
          {filterLabel("Glowstone Dust"), 3, {1}},
          {filterLabel("Gold Ingot"), 1, {3}}
        }, 8},
        {{{filterLabel("Tertius Alchemical Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Lapis Lazuli"), 2, {4}},
          {filterLabel("Alchemical Gold Ingot"), 1, {5}}
        }, 8},
        {{{filterLabel("Nickel Alchemical Ore Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Iron Ingot"), 1, {6}}
        }, 8},
        {{{filterLabel("Osmium Alchemical Ore Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Osmium Dust"), 1, {7}}
        }, 8},
        {{{filterLabel("Cobalt Alchemical Ore Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Cobalt Ore Dust"), 1, {8}}
        }, 8},
        {{{filterLabel("Ardite Alchemical Ore Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Ardite Ore Dust"), 1, {9}}
        }, 8}
      }));

    // alchemicalB
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alchemicalB", "center", "441", Actions::up, Actions::east,
      std::vector<size_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Iron Alchemical Ore Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1}},
          {filterLabel("Pulverized Iron"), 1, {2}}
        }, 8},
        {{{filterLabel("Aluminum Alchemical Ore Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1}},
          {filterLabel("Pulverized Aluminum"), 1, {3}}
        }, 8},
        {{{filterLabel("Crystal Shard"), 16}}, {
          {filterLabel("Glass"), 1, {4}}
        }, 8},
        {{{filterLabel("Silver Alchemical Ore Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1}},
          {filterLabel("Pulverized Silver"), 1, {5}}
        }, 8},
        {{{filterLabel("Copper Alchemical Ore Dust"), 16}}, {
          {filterLabel("Gunpowder"), 2, {6}},
          {filterLabel("Pulverized Copper"), 1, {7}}
        }, 8},
        {{{filterLabel("Lead Alchemical Ore Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1}},
          {filterLabel("Pulverized Lead"), 1, {8}}
        }, 8},
        {{{filterLabel("Tin Alchemical Ore Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1}},
          {filterLabel("Pulverized Tin"), 1, {9}}
        }, 8}
      }));

    // alchemicalC
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alchemicalC", "center", "441", Actions::north, Actions::east,
      std::vector<size_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Dirt"), 64}}, {
          {filterLabel("Plant Matter"), 6, {1}}
        }, 8},
        {{{filterLabel("Stardust"), 64}}, {
          {filterLabel("Aquamarine"), 2, {2}},
          {filterLabel("Iron Ingot"), 1, {3}},
          {filterLabel("Glowstone Dust"), 2, {4}}
        }, 8},
        {{{filterLabel("Uranium Alchemical Ore Dust"), 16}}, {
          {filterLabel("Radioactive Powder Mixture"), 1, {5}},
          {filterLabel("Rotten Flesh"), 1, {6}}
        }, 8}
      }));

    // condenser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "condenser", "center", "d40", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Uranium Ingot"), 64}}, {{filterLabel("Uranium Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Gold Ingot"), 64}}, {{filterLabel("Gold Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Osmium Ingot"), 64}}, {{filterLabel("Osmium Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Ardite Ingot"), 64}}, {{filterLabel("Ardite Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Iron Ingot"), 64}}, {{filterLabel("Iron Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Aluminum Ingot"), 64}}, {{filterLabel("Aluminum Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Nickel Ingot"), 64}}, {{filterLabel("Nickel Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Silver Ingot"), 64}}, {{filterLabel("Silver Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Copper Ingot"), 64}}, {{filterLabel("Copper Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Lead Ingot"), 64}}, {{filterLabel("Lead Alchemical Ore Dust"), 1, {0}}}, 2},
        {{{filterLabel("Tin Ingot"), 64}}, {{filterLabel("Tin Alchemical Ore Dust"), 1, {0}}}, 2}
      }));

    // manufactory
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manufactory", "center", "258", Actions::south, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Flint"), 64}}, {{filterLabel("Gravel"), 1, {0}}}, 16},
        {{{filterLabel("Sand"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Bone Meal"), 64}}, {{filterLabel("Bone"), 1, {0}}}, 16},
        {{{filterLabel("Silicon Ingot"), 64}}, {{filterLabel("Sand"), 1, {0}}}, 16},
        {{{filterLabel("Osmium Dust"), 16}}, {{filterLabel("Osmium Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Cobalt Ore Dust"), 16}}, {{filterLabel("Cobalt Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Ardite Ore Dust"), 16}}, {{filterLabel("Ardite Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Iron"), 16}}, {{filterLabel("Iron Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Aluminum"), 16}}, {{filterLabel("Aluminum Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Silver"), 16}}, {{filterLabel("Silver Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Copper"), 16}}, {{filterLabel("Copper Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Lead"), 16}}, {{filterLabel("Lead Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Tin"), 16}}, {{filterLabel("Tin Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Coal"), 64}}, {{filterLabel("Coal"), 1, {0}}}, 16},
        {{{filterLabel("Clay Dust"), 4}}, {{filterName("minecraft:clay"), 1, {0}}}, 16}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "furnace", "center", "d40", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Glass"), 64}}, {{filterLabel("Sand"), 1, {0}}}, 16},
        {{{filterLabel("Stone"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16},
        {{{filterLabel("Graphite Ingot"), 64}}, {{filterLabel("Charcoal"), 1, {0}}}, 16},
        {{{filterLabel("Plastic"), 64}}, {{filterLabel("Dry Rubber"), 1, {0}}}, 16}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "north", "ab7", Actions::east, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Bronze Ingot"), 64}}, {
          {filterLabel("Copper Ingot"), 3, {0}},
          {filterLabel("Tin Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Steel Ingot"), 64}}, {
          {filterLabel("Iron Ingot"), 1, {0}},
          {filterLabel("Graphite Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Invar Ingot"), 64}}, {
          {filterLabel("Iron Ingot"), 2, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Electrical Steel Ingot"), 64}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Silicon Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Electrum Ingot"), 64}}, {
          {filterLabel("Silver Ingot"), 1, {0}},
          {filterLabel("Gold Ingot"), 1, {1}}
        }, 16}
      }));

    // slimeCrafter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "slimeCrafter", "north", "334", Actions::south, Actions::east,
      std::vector<size_t>{1, 3, 5, 7}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rice Slimeball"), 64}}, {
          {filterLabel("Rice Dough"), 4, {1, 3, 5, 7}}
        }, 16}
      }));

    // mekCrusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "mekCrusher", "center", "441", Actions::south, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Bio Fuel"), 64}}, {{filterLabel("Potato"), 1, {0}}}, 16}
      }));

    // enrichment
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enrichment", "center", "12a", Actions::north, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Compressed Redstone"), 16}}, {{filterLabel("Redstone"), 1, {0}}}, 16},
        {{{filterLabel("Compressed Diamond"), 16}}, {{filterLabel("Diamond"), 1, {0}}}, 16}
      }));

    // redstoneInfusion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneInfusion", "center", "1cb", Actions::down, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enriched Alloy"), 64}}, {{filterLabel("Iron Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Basic Control Circuit"), 64}}, {{filterLabel("Osmium Ingot"), 1, {0}}}, 16}
      }));

    // presser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "presser", "north", "1e4", Actions::down, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Silken Tofu"), 64}}, {{filterLabel("Soybean"), 1, {0}}}, 16},
        {{{filterLabel("Soy Milk"), 64}}, {{filterLabel("Silken Tofu"), 1, {0}}}, 16}
      }));

    // compressedHammer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressedHammer", "north", "1e4", Actions::east, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Dust"), 64}}, {{filterLabel("Compressed Sand"), 1, {0}}}, 16}
      }));

    // impregnatedStick
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "impregnatedStick", "north", "334", Actions::west, Actions::east,
      std::vector<size_t>{16}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Impregnated Stick"), 64}}, {{filterLabel("Birch Wood"), 2, {16}}}, 16}
      }));

    // compressor
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressor", "north", "ab7", Actions::north, Actions::west,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Advanced Alloy"), 4}}, {{filterLabel("Mixed Metal Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Carbon Plate"), 4}}, {{filterLabel("Raw Carbon Mesh"), 1, {6}}}, 16}
      }));

    // cable
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cable", "center", "12a", Actions::down, Actions::south,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Gold Cable"), 64}}, {{filterLabel("Gold Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Copper Cable"), 64}}, {{filterLabel("Copper Ingot"), 1, {6}}}, 16}
      }));

    // itemCasing
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "itemCasing", "center", "5a0", Actions::down, Actions::north,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Tin Item Casing"), 4}}, {{filterLabel("Tin Plate"), 1, {6}}}, 16}
      }));

    // rockCrusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCrusher", "center", "d40", Actions::north, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Gravel"), 64}
        }, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{
          {filterLabel("Dirty Lapis Lazuli"), 16},
          {filterLabel("Dirty Certus Quartz"), 16},
          {filterLabel("Dirty Diamond"), 16},
          {filterLabel("Dirty Emerald"), 16},
          {filterLabel("Dirty Aquamarine"), 16}
        }, {{filterLabel("Stone"), 1, {0}}}, 16},
        {{
          {filterLabel("Dirty Quartz"), 16},
        }, {{filterLabel("Netherrack"), 1, {0}}}, 16}
      }));

    // rockCleaner
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCleaner", "north", "ab7", Actions::south, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Certus Quartz Crystal"), 64}}, {{filterLabel("Dirty Certus Quartz"), 1, {0}}}, 16},
        {{{filterLabel("Lapis Lazuli"), 256}}, {{filterLabel("Dirty Lapis Lazuli"), 1, {0}}}, 16},
        {{{filterLabel("Nether Quartz"), 64}}, {{filterLabel("Dirty Quartz"), 1, {0}}}, 16},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Dirty Diamond"), 1, {0}}}, 16},
        {{{filterLabel("Emerald"), 64}}, {{filterLabel("Dirty Emerald"), 1, {0}}}, 16},
        {{{filterLabel("Aquamarine"), 64}}, {{filterLabel("Dirty Aquamarine"), 1, {0}}}, 16}
      }));

    // sawmill
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "sawmill", "center", "258", Actions::north, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Birch Wood Planks"), 64}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16},
        {{{filterLabel("Stick"), 64}}, {{filterLabel("Birch Wood Planks"), 1, {0}}}, 16}
      }));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "north", "0b8", "ab7", Actions::east, Actions::west, Actions::west,
      std::vector<Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Plant Matter"), 64}}, {
          {filterLabel("Wheat"), 5, {5, 2, 4, 6, 8}}
        }, {12, {}}},
        {{{filterName("minecraft:clay_ball"), 64}}, {
          {filterName("minecraft:clay"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Slimeball"), 64}}, {
          {filterLabel("Slime Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Paper"), 64}}, {
          {filterLabel("Rice"), 3, {1, 5, 9}}
        }, {21, {}}},
        {{{filterLabel("Rice Dough"), 64}}, {
          {filterLabel("Rice"), 3, {1, 2, 4}}
        }, {21, {}}},
        {{{filterLabel("Blaze Powder Block"), 16}}, {
          {filterLabel("Blaze Powder"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Dry Rubber"), 64}}, {
          {filterLabel("Tiny Dry Rubber"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Block of Iron"), 4}}, {
          {filterLabel("Iron Ingot"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Sand"), 64}}, {
          {filterLabel("Sand"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Slime Block"), 64}}, {
          {filterLabel("Rice Slimeball"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Glass Pane"), 64}}, {
          {filterLabel("Glass"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("TNT"), 64}}, {
          {filterLabel("Sand"), 4, {2, 4, 6, 8}},
          {filterLabel("Gunpowder"), 5, {1, 3, 5, 7, 9}}
        }, {12, {}}},
        {{{filterLabel("Sugar"), 64}}, {
          {filterLabel("Agave"), 1, {1}}
        }, {64, NonConsumableInfo{1, 2}}},
        {{{filterLabel("Basic Machine Casing"), 4}}, {
          {filterLabel("Aluminum Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Large Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Mixed Metal Ingot"), 4}}, {
          {filterLabel("Iron Plate"), 3, {1, 2, 3}},
          {filterLabel("Bronze Plate"), 3, {4, 5, 6}},
          {filterLabel("Tin Plate"), 3, {7, 8, 9}}
        }, {21, {}}},
        {{{filterLabel("Raw Carbon Fibre"), 4}}, {
          {filterLabel("Pulverized Coal"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Raw Carbon Mesh"), 4}}, {
          {filterLabel("Raw Carbon Fibre"), 2, {1, 4}}
        }, {32, {}}},
        {{{filterLabel("Advanced Machine Casing"), 4}}, {
          {filterLabel("Steel Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Carbon Plate"), 2, {2, 8}},
          {filterLabel("Advanced Alloy"), 2, {4, 6}},
          {filterLabel("Basic Machine Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Grout"), 64}}, {
          {filterLabel("Sand"), 4, {2, 4, 6, 8}},
          {filterLabel("Gravel"), 4, {1, 3, 7, 9}},
          {filterName("minecraft:clay"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Reinforced Stone"), 64}}, {
          {filterLabel("Grout"), 4, {2, 4, 6, 8}},
          {filterLabel("Stone"), 4, {1, 3, 7, 9}},
          {filterLabel("Clay Dust"), 1, {5}}
        }, {4, {}}},
        {{{filterLabel("Machine Case"), 4}}, {
          {filterLabel("Reinforced Stone"), 4, {1, 3, 7, 9}},
          {filterLabel("Plastic"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Machine Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Basic Coil"), 64}}, {
          {filterLabel("Aluminium Wire"), 4, {2, 4, 6, 8}},
          {filterLabel("Enori Crystal"), 2, {1, 9}},
          {filterLabel("Impregnated Stick"), 3, {3, 5, 7}}
        }, {16, {}}},
        {{{filterLabel("Advanced Coil"), 64}}, {
          {filterLabel("Gold Cable"), 4, {2, 4, 6, 8}},
          {filterLabel("Basic Coil"), 1, {5}},
          {filterLabel("Impregnated Stick"), 2, {3, 7}}
        }, {16, {}}},
        {{{filterLabel("Device Frame"), 4}}, {
          {filterLabel("Glass"), 4, {2, 4, 6, 8}},
          {filterLabel("Tin Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Iron Sheetmetal"), 4}}, {
          {filterLabel("Iron Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Steel Sheetmetal"), 4}}, {
          {filterLabel("Steel Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Iron Mechanical Component"), 4}}, {
          {filterLabel("Iron Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Coil"), 4}}, {
          {filterLabel("Iron Ingot"), 1, {5}},
          {filterLabel("Copper Cable"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Sturdy Casing"), 4}}, {
          {filterLabel("Copper Gear"), 2, {1, 3}},
          {filterLabel("Bronze Gear"), 2, {7, 9}},
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}
        }, {16, {}}}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
