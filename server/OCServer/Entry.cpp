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

    /*
      Fluid:
        White       Liquid Starlight
        Orange      Gelid Cryotheum
        Magenta     Essence of Knowledge
        Light Blue  Energized Glowstone
        Yellow      Resonant Ender
        Lime        Destabilized Redstone
        Pink        Lava
        Gray        Potion of Swiftness
        Light Gray  Awkward Potion
        Cyan        Creosote Oil
        Purple      Latex
        Blue        Water
        Brown       Seed Oil
        Red
        Green
        Black
      Item:
        White
        Orange      output
        Magenta     platePress
        Light Blue  stock
        Yellow      wirePress
        Lime        starlightTransmutation
        Pink        empowerer
        Gray        rosinGen
        Light Gray
        Cyan
        Purple
        Blue
        Brown
        Red
        Green
        Black
    */
    auto blueSlime(filterFn([](const Item &item) {
      return item.name == "tconstruct:edible" && item.damage == 1;
    }));

    auto congealedBlueSlime(filterFn([](const Item &item) {
      return item.name == "tconstruct:slime_congealed" && item.damage == 1;
    }));

    Factory factory(server, 1000, "south", {{"center", "127", Actions::up}, {"south", "fff", Actions::down}});
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "7f4", Actions::south, Actions::down));
    factory.addStorage(std::make_unique<StorageChest>(factory, "south", "fff", Actions::up, Actions::down));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "127", Actions::down, Actions::up, std::vector<SharedItemFilter>{
      filterLabel("Birch Sapling"), filterLabel("Experience Seeds"), filterLabel("Seeds"), filterLabel("Wheat"),
      filterName("mysticalagriculture:tier4_inferium_seeds"), filterLabel("Lead Ore Piece"), filterLabel("Substrate"), filterLabel("Nickel Ingot"),
      filterLabel("Firm Tofu"), filterLabel("Sawdust"), filterLabel("Redstone Seeds"), filterLabel("Aluminium Ore Piece"),
      filterLabel("Nickel Ore Piece"), filterLabel("Magnesium Ore Piece"), filterLabel("Chorus Flower"), filterLabel("Sky Stone Dust"),
      filterLabel("Crushed Black Quartz"), filterLabel("Iron Ore Piece"), filterLabel("Tin Ore Piece"), filterLabel("Gold Ore Piece"),
      filterLabel("Osmium Ore Piece"), filterLabel("Silver Ore Piece"), filterLabel("Copper Ore Piece"), filterLabel("Veggie Bait"),
      filterLabel("Pumpkin Seeds"), filterLabel("Grain Bait"), filterLabel("Poisonous Potato"), filterLabel("Basalt Seeds"),
      filterLabel("Coal Coke"), filterLabel("Lapis Lazuli Seeds"), filterLabel("Grains of Infinity"), filterLabel("Thorium Ore Piece"),
      filterLabel("Lithium Ore Piece"), filterLabel("Cobalt Ore Piece"), filterLabel("Ardite Ore Piece"), filterLabel("Boron Ore Piece"),
      filterLabel("Diamond"), filterLabel("Emerald"), filterLabel("Soy Milk"), filterLabel("Ruby"),
      filterLabel("Sapphire"), filterLabel("Malachite"), filterLabel("Amber"), filterLabel("Peridot"),
      filterLabel("Tanzanite"), filterLabel("Topaz"), filterLabel("Dye Seeds"), filterLabel("Cyanite Ingot"),
      filterLabel("Fiery Ingot Seeds"), filterLabel("Crushed Fluorite"), filterLabel("Crushed Carobbiite"), filterLabel("Zirconium Dust"),
      filterLabel("Melon Seeds")
    }));
    factory.addBackup(filterLabel("Sweet Potato"), 32);
    factory.addBackup(filterLabel("Sugar Canes"), 8);
    factory.addBackup(filterLabel("Nether Wart"), 8);
    factory.addBackup(filterLabel("Netherrack"), 32);
    factory.addBackup(filterLabel("Cranberry"), 32);
    factory.addBackup(filterLabel("Dandelion"), 8);
    factory.addBackup(filterLabel("Gunpowder"), 8);
    factory.addBackup(filterLabel("Redstone"), 32);
    factory.addBackup(filterLabel("Soybean"), 32);
    factory.addBackup(filterLabel("Potato"), 32);
    factory.addBackup(filterLabel("Cactus"), 8);
    factory.addBackup(filterLabel("Sulfur"), 8);
    factory.addBackup(filterLabel("Onion"), 32);
    factory.addBackup(filterLabel("Seeds"), 32);
    factory.addBackup(filterLabel("Flax"), 32);
    factory.addBackup(filterLabel("Corn"), 32);

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "7f4", Actions::east, Actions::down,
      std::vector<StockEntry>{}, INT_MAX, nullptr, outAll, std::vector<Recipe<int>>{}));

    // stock
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "stock", "center", "7f4", Actions::west, Actions::down,
      std::vector<StockEntry>{
        {filterLabel("Compressed Redstone"), 16},
        {filterLabel("Compressed Obsidian"), 16},
        {filterLabel("Compressed Diamond"), 16},
        {filterLabel("Experience Essence"), 16},
        {filterLabel("Glistering Melon"), 16},
        {filterLabel("Fluxed Phyto-Gro"), 16},
        {filterLabel("Glowstone Dust"), 16},
        {filterLabel("Cryotheum Dust"), 16},
        {filterLabel("Basalz Powder"), 16},
        {filterLabel("Uranium Ingot"), 16},
        {filterLabel("Lumium Ingot"), 16},
        {filterLabel("Cobblestone"), 16},
        {filterLabel("Nether Wart"), 16},
        {filterLabel("Ender Pearl"), 16},
        {filterLabel("Birch Wood"), 16},
        {filterLabel("Aquamarine"), 16},
        {filterLabel("Netherrack"), 64},
        {filterLabel("Bio Fuel"), 16},
        {filterLabel("Charcoal"), 16},
        {filterLabel("Redstone"), 16},
        {filterLabel("Sugar"), 16},
        {filterLabel("Seeds"), 16},
        {filterLabel("Sand"), 16},
        {filterLabel("Coal"), 16},
        {filterLabel("Cake"), 1}
      }, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{}));

    // enderChest
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "enderChest", "south", "fff", Actions::east, Actions::down,
      std::vector<StockEntry>{
        {filterLabel("Factory Block"), 64 * 9},
        {filterLabel("Glass"), 64 * 9},
        {filterLabel("Potion of Swiftness"), 9},
        {filterLabel("Potion of Healing"), 9},
        {filterLabel("Potion of Haste"), 9},
        {filterLabel("Thankful Dinner"), 64 * 9}
      }, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{}));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "cobbleGen", "center", "127", Actions::south, Actions::up,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 128)));

    // rubberGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "rubberGen", "center", "7a5", Actions::up, Actions::down,
      6, ProcessInputless::makeNeeded(factory, filterLabel("Tiny Dry Rubber"), 128)));

    // obsidianGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "obsidianGen", "south", "06e", Actions::down, Actions::up,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Obsidian"), 128)));

    // waterGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "waterGen", "center", "0ed", Actions::west, Actions::north,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Fresh Water"), 128)));

    // snowballGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "snowballGen", "south", "cb8", Actions::east, Actions::down,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Snowball"), 128)));

    // rosinGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "rosinGen", "south", "547", Actions::south, Actions::down,
      1, ProcessInputless::makeNeeded(factory, filterLabel("Rosin"), 128)));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "center", "1dc", "ffd",
      Actions::south, Actions::south, Actions::up,
      std::vector<Recipe<std::pair<int, std::vector<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Button"), 16}}, {
          {filterLabel("Stone"), 1, {1}}
        }, {64, {}}},
        {{{filterName("minecraft:clay_ball"), 16}}, {
          {filterName("minecraft:clay"), 1, {1}}
        }, {16, {}}},
        {{{filterLabel("Quartz Sliver"), 16}}, {
          {filterLabel("Nether Quartz"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Chest"), 16}}, {
          {filterLabel("Birch Wood"), 4, {1, 3, 7, 9}},
          {filterLabel("Treated Wood Planks"), 4, {2, 4, 6, 8}},
          {filterLabel("Button"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Arcane Stone"), 16}}, {
          {filterLabel("Stone"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Herba Vis Crystal"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Redstone Torch"), 16}}, {
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Stick"), 1, {4}}
        }, {64, {}}},
        {{{filterLabel("Grout"), 16}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Gravel"), 1, {2}},
          {filterName("minecraft:clay_ball"), 1, {3}}
        }, {32, {}}},
        {{{filterLabel("Reinforced Stone"), 16}}, {
          {filterLabel("Stone"), 4, {1, 3, 7, 9}},
          {filterLabel("Grout"), 4, {2, 4, 6, 8}},
          {filterLabel("Clay Dust"), 1, {5}}
        }, {4, {}}},
        {{{filterLabel("Basic Machine Casing"), 1}}, {
          {filterLabel("Aluminum Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Large Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Torch"), 16}}, {
          {filterLabel("Charcoal"), 1, {1}},
          {filterLabel("Stick"), 1, {4}}
        }, {16, {}}},
        {{{filterLabel("Raw Carbon Fibre"), 8}}, {
          {filterLabel("Pulverized Coal"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Raw Carbon Mesh"), 8}}, {
          {filterLabel("Raw Carbon Fibre"), 2, {1, 4}}
        }, {32, {}}},
        {{{filterLabel("Mixed Metal Ingot"), 8}}, {
          {filterLabel("Iron Plate"), 3, {1, 2, 3}},
          {filterLabel("Bronze Plate"), 3, {4, 5, 6}},
          {filterLabel("Tin Plate"), 3, {7, 8, 9}}
        }, {21, {}}},
        {{{filterLabel("Advanced Machine Casing"), 2}}, {
          {filterLabel("Steel Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Carbon Plate"), 2, {2, 8}},
          {filterLabel("Advanced Alloy"), 2, {4, 6}},
          {filterLabel("Basic Machine Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Machine Case"), 1}}, {
          {filterLabel("Reinforced Stone"), 4, {1, 3, 7, 9}},
          {filterLabel("Plastic"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Machine Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Iron Sheetmetal"), 8}}, {
          {filterLabel("Iron Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Lead Sheetmetal"), 8}}, {
          {filterLabel("Lead Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Steel Sheetmetal"), 8}}, {
          {filterLabel("Steel Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Aluminium Sheetmetal"), 8}}, {
          {filterLabel("Aluminum Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Iron Bars"), 16}}, {
          {filterLabel("Iron Ingot"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("Glass Pane"), 16}}, {
          {filterLabel("Glass"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("String"), 16}}, {
          {filterLabel("Flax"), 3, {1, 2, 4}}
        }, {21, {}}},
        {{{filterLabel("Piston"), 8}}, {
          {filterLabel("Treated Wood Planks"), 3, {1, 2, 3}},
          {filterLabel("Compressed Cobblestone"), 4, {4, 6, 7, 9}},
          {filterLabel("Iron Plate"), 1, {5}},
          {filterLabel("Redstone"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Coil"), 8}}, {
          {filterLabel("Copper Cable"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Electric Motor"), 8}}, {
          {filterLabel("Tin Item Casing"), 2, {2, 8}},
          {filterLabel("Coil"), 2, {4, 6}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Heat Vent"), 4}}, {
          {filterLabel("Iron Bars"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Plate"), 4, {2, 4, 6, 8}},
          {filterLabel("Electric Motor"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Range Addon"), 1}}, {
          {filterLabel("Cobblestone"), 6, {1, 3, 4, 6, 7, 9}},
          {filterLabel("Plastic"), 2, {2, 8}},
          {filterLabel("Glass Pane"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Sturdy Casing"), 1}}, {
          {filterLabel("Copper Gear"), 2, {1, 3}},
          {filterLabel("Bronze Gear"), 2, {7, 9}},
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Iron Casing"), 2}}, {
          {filterLabel("Iron Sheetmetal"), 4, {1, 3, 7, 9}},
          {filterLabel("Tin Electron Tube"), 4, {2, 4, 6, 8}},
          {filterLabel("Hardened Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Device Frame"), 1}}, {
          {filterLabel("Tin Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Glass"), 4, {2, 4, 6, 8}},
          {filterLabel("Copper Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Sandstone"), 16}}, {
          {filterLabel("Sand"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Clock"), 8}}, {
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Rich Phyto-Gro"), 16}}, {
          {filterLabel("Pulverized Charcoal"), 1, {1}},
          {filterLabel("Niter"), 1, {2}},
          {filterLabel("Rich Slag"), 1, {4}}
        }, {4, {}}},
        {{{filterLabel("Basic Plating"), 8}}, {
          {filterLabel("Lead Sheetmetal"), 4, {1, 3, 7, 9}},
          {filterLabel("Lead Item Casing"), 4, {2, 4, 6, 8}},
          {filterLabel("Graphite Block"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Advanced Plating"), 8}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Tough Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Basic Plating"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Plain Yogurt"), 16}}, {
          {filterLabel("Soy Milk"), 1, {1}}
        }, {64, {{1, 9}}}},
        {{{filterLabel("Marshmallows"), 16}}, {
          {filterLabel("Sugar"), 1, {1}},
          {filterLabel("Fresh Water"), 1, {2}},
          {filterLabel("Raw Tofeeg"), 1, {3}}
        }, {64, {{1, 9}}}},
        {{{filterLabel("Salt"), 16}}, {
          {filterLabel("Fresh Water"), 1, {1}}
        }, {64, {{1, 9}}}},
        {{{filterLabel("Dough"), 16}}, {
          {filterLabel("Fresh Water"), 1, {1}},
          {filterName("harvestcraft:flouritem"), 1, {2}},
          {filterLabel("Salt"), 1, {3}}
        }, {64, {{4, 9}}}},
        {{{filterLabel("Sweet Potato Pie"), 16}}, {
          {filterLabel("Sweet Potato"), 1, {1}},
          {filterLabel("Dough"), 1, {2}},
          {filterLabel("Ground Cinnamon"), 1, {3}},
          {filterLabel("Marshmallows"), 1, {4}}
        }, {64, {{3, 9}}}},
        {{{filterLabel("Toast"), 16}}, {
          {filterLabel("Bread"), 1, {1}},
          {filterLabel("Butter"), 1, {2}}
        }, {64, {{3, 9}}}},
        {{{filterLabel("Raw Tofurkey"), 16}}, {
          {filterLabel("Firm Tofu"), 1, {1}},
          {filterLabel("Cooking Oil"), 1, {2}},
          {filterLabel("Bread"), 1, {3}}
        }, {12, {{2, 9}}}},
        {{{filterLabel("Thankful Dinner"), 16}}, {
          {filterLabel("Cooked Tofurkey"), 1, {1}},
          {filterLabel("Mashed Potatoes"), 1, {2}},
          {filterLabel("Sweet Potato Pie"), 1, {3}},
          {filterLabel("Cranberry Jelly"), 1, {4}},
          {filterLabel("Corn"), 1, {5}},
          {filterLabel("Onion"), 1, {6}},
          {filterLabel("Toast"), 1, {7}}
        }, {12, {{2, 9}}}},
        {{{filterLabel("Cranberry Jelly"), 16}}, {
          {filterLabel("Cranberry"), 1, {1}},
          {filterLabel("Sugar"), 1, {2}}
        }, {64, {{6, 9}}}},
        {{{filterLabel("Butter"), 16}}, {
          {filterLabel("Silken Tofu"), 1, {1}},
          {filterLabel("Salt"), 1, {2}}
        }, {64, {{6, 9}}}},
        {{{filterLabel("Mashed Potatoes"), 16}}, {
          {filterLabel("Buttered Potato"), 1, {1}},
          {filterLabel("Salt"), 1, {2}}
        }, {64, {{4, 9}}}},
        {{{filterLabel("Buttered Potato"), 16}}, {
          {filterLabel("Baked Potato"), 1, {1}},
          {filterLabel("Butter"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Paper"), 16}}, {
          {filterLabel("Sugar Canes"), 3, {1, 2, 3}}
        }, {21, {}}},
        {{{filterLabel("Advanced Control Circuit"), 8}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Enriched Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Basic Control Circuit"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Elite Control Circuit"), 8}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Reinforced Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Control Circuit"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Raw Tofeeg"), 16}}, {
          {filterLabel("Firm Tofu"), 1, {1}},
          {filterLabel("Dandelion Yellow"), 1, {2}}
        }, {32, {{2, 9}}}},
        {{{filterLabel("Basic Coil"), 8}}, {
          {filterLabel("Impregnated Stick"), 3, {3, 5, 7}},
          {filterLabel("Enori Crystal"), 2, {1, 9}},
          {filterLabel("Aluminium Wire"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Advanced Coil"), 8}}, {
          {filterLabel("Impregnated Stick"), 2, {3, 7}},
          {filterLabel("Basic Coil"), 1, {5}},
          {filterLabel("Gold Cable"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterName("minecraft:melon"), 16}}, {
          {filterName("minecraft:melon_block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Pulsating Iron Nugget"), 16}}, {
          {filterLabel("Pulsating Iron Ingot"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Iron Nugget"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Gold Nugget"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Restonia Crystal"), 16}}, {
          {filterLabel("Empowered Restonia Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Palis Crystal"), 16}}, {
          {filterLabel("Empowered Palis Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Diamatine Crystal"), 16}}, {
          {filterLabel("Empowered Diamatine Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Void Crystal"), 16}}, {
          {filterLabel("Empowered Void Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Emeradic Crystal"), 16}}, {
          {filterLabel("Empowered Emeradic Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Empowered Enori Crystal"), 16}}, {
          {filterLabel("Empowered Enori Crystal Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Diamond Nugget"), 16}}, {
          {filterLabel("Diamond"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Hopper"), 8}}, {
          {filterLabel("Chest"), 1, {5}},
          {filterLabel("Iron Plate"), 5, {1, 3, 4, 6, 8}}
        }, {12, {}}},
        {{{filterLabel("Item Interface"), 4}}, {
          {filterLabel("Chest"), 1, {5}},
          {filterLabel("Basic Coil"), 4, {1, 3, 7, 9}},
          {filterLabel("Redstone"), 2, {2, 8}},
          {filterLabel("Restonia Crystal"), 2, {4, 6}}
        }, {16, {}}},
        {{{filterLabel("Raw Circuit Board"), 8}}, {
          {filterLabel("Gold Ingot"), 1, {1}},
          {filterName("minecraft:clay_ball"), 1, {2}},
          {filterLabel("Cactus Green"), 1, {3}}
        }, {8, {}}},
        {{{filterLabel("Glass Bottle"), 16}}, {
          {filterLabel("Glass"), 3, {1, 3, 5}}
        }, {21, {}}},
        {{{filterLabel("Bucket"), 8}}, {
          {filterLabel("Iron Plate"), 3, {1, 3, 5}}
        }, {21, {}}},
        {{{filterLabel("Transistor"), 8}}, {
          {filterLabel("Iron Ingot"), 3, {1, 2, 3}},
          {filterLabel("Gold Nugget"), 2, {4, 6}},
          {filterLabel("Paper"), 1, {5}},
          {filterLabel("Redstone"), 1, {8}}
        }, {8, {}}},
        {{{filterLabel("Analyzer"), 4}}, {
          {filterLabel("Redstone Torch"), 1, {1}},
          {filterLabel("Gold Nugget"), 2, {5, 8}},
          {filterLabel("Transistor"), 1, {4}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {7}}
        }, {32, {}}},
        {{{filterLabel("Dropper"), 8}}, {
          {filterLabel("Cobblestone"), 7, {1, 2, 3, 4, 6, 7, 9}},
          {filterLabel("Redstone"), 1, {8}}
        }, {9, {}}},
        {{{filterLabel("Dispenser"), 8}}, {
          {filterLabel("Cobblestone"), 7, {1, 2, 3, 4, 6, 7, 9}},
          {filterLabel("Redstone"), 1, {8}},
          {filterLabel("String"), 1, {5}}
        }, {9, {}}},
        {{{filterLabel("Microchip (Tier 1)"), 8}}, {
          {filterLabel("Iron Nugget"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Transistor"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Microchip (Tier 2)"), 8}}, {
          {filterLabel("Gold Nugget"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Transistor"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Microchip (Tier 3)"), 8}}, {
          {filterLabel("Diamond Nugget"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Transistor"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Memory (Tier 3.5)"), 4}}, {
          {filterLabel("Microchip (Tier 3)"), 3, {1, 2, 3}},
          {filterLabel("Microchip (Tier 2)"), 2, {4, 6}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {5}}
        }, {21, {}}},
        {{{filterLabelName("Card Base", "opencomputers:material"), 8}}, {
          {filterLabel("Iron Nugget"), 3, {1, 4, 7}},
          {filterLabel("Gold Nugget"), 1, {8}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {5}}
        }, {21, {}}},
        {{{filterLabel("Inventory Controller Upgrade"), 4}}, {
          {filterLabel("Gold Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {8}},
          {filterLabel("Microchip (Tier 2)"), 1, {5}},
          {filterLabel("Piston"), 1, {6}},
          {filterLabel("Analyzer"), 1, {2}},
          {filterLabel("Dropper"), 1, {4}}
        }, {16, {}}},
        {{{filterLabel("Tank Controller Upgrade"), 4}}, {
          {filterLabel("Gold Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {8}},
          {filterLabel("Microchip (Tier 2)"), 1, {5}},
          {filterLabel("Piston"), 1, {6}},
          {filterLabel("Glass Bottle"), 1, {2}},
          {filterLabel("Dispenser"), 1, {4}}
        }, {16, {}}},
        {{{filterLabel("Transposer"), 4}}, {
          {filterLabel("Iron Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Inventory Controller Upgrade"), 1, {2}},
          {filterLabel("Tank Controller Upgrade"), 1, {8}},
          {filterLabel("Hopper"), 2, {4, 6}},
          {filterLabel("Bucket"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Gunpowder"), 16}}, {
          {filterLabel("Niter"), 4, {1, 2, 3, 4}},
          {filterLabel("Sulfur"), 1, {5}, true},
          {filterLabel("Charcoal"), 1, {6}}
        }, {16, {}}},
        {{{filterName("rftools:machine_frame"), 1}}, {
          {filterLabel("Heat Vent"), 2, {1, 3}},
          {filterLabel("Dry Rubber"), 2, {4, 6}},
          {filterLabel("Pink Slime"), 2, {7, 9}},
          {filterLabel("Gold Gear"), 1, {2}},
          {filterLabel("Machine Case"), 1, {5}},
          {filterLabel("Range Addon"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Steel Scaffolding"), 8}}, {
          {filterLabel("Steel Ingot"), 3, {1, 2, 3}},
          {filterLabel("Steel Rod"), 3, {5, 7, 9}}
        }, {10, {}}},
        {{{filterLabel("Reinforced Blast Brick"), 8}}, {
          {filterLabel("Steel Plate"), 1, {1}},
          {filterLabel("Blast Brick"), 1, {4}}
        }, {64, {}}},
        {{{filterLabel("Blast Brick"), 8}}, {
          {filterName("minecraft:netherbrick"), 4, {1, 3, 7, 9}},
          {filterLabel("Brick"), 4, {2, 4, 6, 8}},
          {filterLabel("Blaze Powder"), 1, {5}}
        }, {16, {}}},
        {{{filterName("minecraft:nether_brick"), 16}}, {
          {filterName("minecraft:netherbrick"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Iron Mechanical Component"), 8}}, {
          {filterLabel("Iron Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Steel Mechanical Component"), 8}}, {
          {filterLabel("Steel Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Steel Casing"), 1}}, {
          {filterLabel("Osmium Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Osmium Block"), 2, {2, 8}},
          {filterLabel("Steel Mechanical Component"), 2, {4, 6}},
          {filterLabel("Reinforced Blast Brick"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Stone Gear"), 8}}, {
          {filterLabel("Cobblestone"), 5, {5, 2, 4, 6, 8}}
        }, {12, {}}},
        {{{filterLabel("Furnace"), 8}}, {
          {filterLabel("Compressed Cobblestone"), 4, {1, 3, 7, 9}},
          {filterLabel("Cobblestone"), 2, {2, 8}},
          {filterLabel("Stone Gear"), 2, {4, 6}},
          {filterLabel("Charcoal"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("LV Wire Coil"), 8}}, {
          {filterLabel("Copper Wire"), 4, {2, 4, 6, 8}},
          {filterLabel("Stick"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Copper Coil Block"), 4}}, {
          {filterLabel("LV Wire Coil"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Factory Block"), 16}}, {
          {filterLabel("Stone"), 4, {2, 4, 6, 8}},
          {filterLabel("Iron Ingot"), 4, {1, 3, 7, 9}}
        }, {2, {}}},
        {{{filterLabel("Heavy Engineering Block"), 1}}, {
          {filterLabel("Uranium Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Mechanical Component"), 2, {4, 6}},
          {filterLabel("Reinforced Alloy"), 2, {2, 8}},
          {filterLabel("Steel Scaffolding"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Engineering Block"), 1}}, {
          {filterLabel("Copper Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Constantan Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Gear"), 1}}, {
          {filterLabel("Redstone Torch"), 4, {2, 4, 6, 8}},
          {filterLabel("Birch Wood Planks"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Energetic Blend"), 16}}, {
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Glowstone Dust"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel(u8"§aPrudentium Essence"), 36}}, {
          {filterLabel(u8"§eInferium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{5, 5}}}},
        {{{filterLabel(u8"§6Intermedium Essence"), 36}}, {
          {filterLabel(u8"§aPrudentium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{5, 5}}}},
        {{{filterLabel(u8"§bSuperium Essence"), 36}}, {
          {filterLabel(u8"§6Intermedium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{5, 5}}}},
        {{{filterLabel(u8"§cSupremium Essence"), 36}}, {
          {filterLabel(u8"§bSuperium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{5, 5}}}},
        {{{filterLabel("Redstone Servo"), 8}}, {
          {filterLabel("Redstone"), 2, {2, 8}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Pyrotheum Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {3}},
          {filterLabel("Sulfur"), 1, {4}}
        }, {32, {}}},
        {{{filterLabel("Cryotheum Dust"), 16}}, {
          {filterLabel("Blizz Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {3}},
          {filterLabel("Snowball"), 1, {4}}
        }, {32, {}}},
        {{{filterLabel("Lapis Lazuli"), 64}}, {
          {filterLabel("Lapis Lazuli Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
        {{{filterLabel("Fiery Ingot"), 16}}, {
          {filterLabel("Fiery Ingot Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Energy Laser Relay"), 8}}, {
          {filterLabel("Obsidian"), 4, {1, 3, 7, 9}},
          {filterLabel("Block of Redstone"), 2, {2, 8}},
          {filterLabel("Restonia Crystal"), 2, {4, 6}},
          {filterLabel("Advanced Coil"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Red Nether Brick"), 16}}, {
          {filterName("minecraft:netherbrick"), 2, {1, 5}},
          {filterLabel("Nether Wart"), 2, {2, 4}}
        }, {32, {}}},
        {{{filterLabel("Ink Sac"), 16}}, {
          {filterLabel("Dye Essence"), 3, {1, 2, 3}}
        }, {16, {}}},
        {{{filterLabel("Basalt"), 16}}, {
          {filterLabel("Basalt Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {2, {}}},
        {{{filterLabel("Pulsating Crystal"), 16}}, {
          {filterLabel("Pulsating Iron Nugget"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Diamond"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Glistering Melon"), 16}}, {
          {filterLabel("Gold Nugget"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterName("minecraft:melon"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Energium Dust"), 16}}, {
          {filterLabel("Redstone"), 5, {1, 3, 5, 7, 9}},
          {filterLabel("Diamond Dust"), 4, {2, 4, 6, 8}}
        }, {7, {}}},
        {{{filterLabel("Reactor Casing Core (Legacy)"), 16}}, {
          {filterLabel("Ferroboron Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Gold Plate"), 2, {4, 6}},
          {filterLabel("Hard Carbon Alloy"), 2, {2, 8}},
          {filterLabel("Block of Redstone"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Reactor Casing (Legacy)"), 16}}, {
          {filterLabel("Steel Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Graphite Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Reactor Casing Core (Legacy)"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Luminessence"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {4}},
          {filterLabel("Gunpowder"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Insulated Tin Cable"), 16}}, {
          {filterLabel("Tin Cable"), 1, {1}},
          {filterLabel("Plastic"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Insulated Copper Cable"), 16}}, {
          {filterLabel("Copper Cable"), 1, {1}},
          {filterLabel("Plastic"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Reinforced Cell Frame (Empty)"), 1}}, {
          {filterLabel("Silver Gear"), 2, {1, 3}},
          {filterLabel("Redstone Conductance Coil"), 1, {2}},
          {filterLabel("Fluxed Electrum Plate"), 2, {4, 6}},
          {filterLabel("Hardened Cell Frame"), 1, {5}},
          {filterLabel("Flux Crystal"), 2, {7, 9}},
          {filterLabel("Electrum Large Plate"), 1, {8}}
        }, {32, {}}},
        {{{filterLabel("Signalum Cell Frame (Empty)"), 1}}, {
          {filterLabel("Signalum Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Cinnabar"), 1, {2}},
          {filterLabel("Rosin"), 2, {4, 6}},
          {filterLabel("Reinforced Cell Frame (Full)"), 1, {5}},
          {filterLabel("Rich Slag"), 1, {8}}
        }, {16, {}}},
        {{{congealedBlueSlime, 16}}, {
          {blueSlime, 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Diorite"), 16}}, {
          {filterLabel("Cobblestone"), 2, {1, 5}},
          {filterLabel("Nether Quartz"), 2, {2, 4}}
        }, {32, {}}}
      }));

    // autoCompressor
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "autoCompressor", "center", "7b7", Actions::west, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Tin Ore"), 8}}, {{filterLabel("Tin Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Iron Ore"), 8}}, {{filterLabel("Iron Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Gold Ore"), 8}}, {{filterLabel("Gold Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Lead Ore"), 8}}, {{filterLabel("Lead Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Redstone"), 256}}, {{filterLabel("Redstone Essence"), 9}}, INT_MAX},
        {{{filterLabel("Boron Ore"), 8}}, {{filterLabel("Boron Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Cobalt Ore"), 8}}, {{filterLabel("Cobalt Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Ardite Ore"), 8}}, {{filterLabel("Ardite Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Nickel Ore"), 8}}, {{filterLabel("Nickel Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Copper Ore"), 8}}, {{filterLabel("Copper Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Silver Ore"), 8}}, {{filterLabel("Silver Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Osmium Ore"), 8}}, {{filterLabel("Osmium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Dry Rubber"), 8}}, {{filterLabel("Tiny Dry Rubber"), 9}}, INT_MAX},
        {{{filterLabel("Uranium Ore"), 8}}, {{filterLabel("Uranium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Lithium Ore"), 8}}, {{filterLabel("Lithium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Thorium Ore"), 8}}, {{filterLabel("Thorium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Aluminum Ore"), 8}}, {{filterLabel("Aluminium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Osmium Block"), 8}}, {{filterLabel("Osmium Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Magnesium Ore"), 8}}, {{filterLabel("Magnesium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Block of Iron"), 8}}, {{filterLabel("Iron Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Invar"), 8}}, {{filterLabel("Invar Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Graphite Block"), 8}}, {{filterLabel("Graphite Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Quartz"), 8}}, {{filterLabel("Nether Quartz"), 4}}, INT_MAX},
        {{{filterLabel("Compressed Dust"), 8}}, {{filterLabel("Dust"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Sand"), 8}}, {{filterLabel("Sand"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Gravel"), 8}}, {{filterLabel("Gravel"), 9}}, INT_MAX},
        {{{filterLabel("Green Slime Block"), 8}}, {{filterName("minecraft:slime_ball"), 9}}, INT_MAX},
        {{{filterLabel("Block of Electrum"), 8}}, {{filterLabel("Electrum Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Redstone"), 40}}, {{filterLabel("Redstone"), 9}}, INT_MAX},
        {{{filterLabel("Void Crystal Block"), 8}}, {{filterLabel("Void Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Palis Crystal Block"), 8}}, {{filterLabel("Palis Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Enori Crystal Block"), 8}}, {{filterLabel("Enori Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Block of Black Iron"), 8}}, {{filterLabel("Black Iron Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Soul Sand"), 8}}, {{filterLabel("Soul Sand"), 9}}, INT_MAX},
        {{{filterLabel("Compressed End Stone"), 8}}, {{filterLabel("End Stone"), 9}}, INT_MAX},
        {{{filterLabel("Block of Black Quartz"), 8}}, {{filterLabel("Black Quartz"), 4}}, INT_MAX},
        {{{filterLabel("Compressed Netherrack"), 8}}, {{filterLabel("Netherrack"), 9}}, INT_MAX},
        {{{filterLabel("Restonia Crystal Block"), 8}}, {{filterLabel("Restonia Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Emeradic Crystal Block"), 8}}, {{filterLabel("Emeradic Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Cobblestone"), 8}}, {{filterLabel("Cobblestone"), 9}}, INT_MAX},
        {{{filterLabel("Diamatine Crystal Block"), 8}}, {{filterLabel("Diamatine Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Ender Gravel"), 8}}, {{filterLabel("Crushed Endstone"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Nether Gravel"), 8}}, {{filterLabel("Crushed Netherrack"), 9}}, INT_MAX}
      }));

    // pureDaisy
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pureDaisy", "center", "7a5", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 8, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Livingwood"), 16}}, {{filterLabel("Infused Wood"), 1}}, INT_MAX},
        {{{filterLabel("Livingrock"), 16}}, {{filterLabel("Arcane Stone"), 1}}, INT_MAX}
      }));

    // waterBarrel
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "waterBarrel", "center", "80c", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:clay"), 16}}, {{filterLabel("Dust"), 1}}, INT_MAX}
      }));

    // smallPlatePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "smallPlatePress", "center", "80c", Actions::north, Actions::down,
      std::vector<StockEntry>{}, 8, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Rod"), 16}}, {{filterLabel("Aluminium Sheetmetal"), 1}}, INT_MAX},
        {{{filterLabel("Steel Rod"), 16}}, {{filterLabel("Steel Sheetmetal"), 1}}, INT_MAX}
      }));

    // atomicReconstructor
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "atomicReconstructor", "center", "80c", Actions::west, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Diamatine Crystal"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Restonia Crystal"), 16}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Emeradic Crystal"), 16}}, {{filterLabel("Emerald"), 1}}, INT_MAX},
        {{{filterLabel("Enori Crystal"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Palis Crystal"), 16}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
        {{{filterLabel("Rhodochrosite"), 16}}, {{filterLabel("Ruby"), 1}}, INT_MAX},
        {{{filterLabel("Void Crystal"), 16}}, {{filterLabel("Coal"), 1}}, INT_MAX},
        {{{filterLabel("Soul Sand"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX}
      }));

    // platePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "platePress", "center", "312", Actions::south, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Tin Plate"), 16}}, {{filterLabel("Tin Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Lead Plate"), 16}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Plate"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Gold Plate"), 16}}, {{filterLabel("Gold Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Steel Plate"), 16}}, {{filterLabel("Steel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Invar Plate"), 16}}, {{filterLabel("Invar Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Plate"), 16}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bronze Plate"), 16}}, {{filterLabel("Bronze Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Plate"), 16}}, {{filterLabel("Uranium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Plate"), 16}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Signalum Plate"), 16}}, {{filterLabel("Signalum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Large Plate"), 4}}, {{filterLabel("Block of Iron"), 1}}, INT_MAX},
        {{{filterLabel("Electrum Large Plate"), 4}}, {{filterLabel("Block of Electrum"), 1}}, INT_MAX},
        {{{filterLabel("Fluxed Electrum Plate"), 16}}, {{filterLabel("Fluxed Electrum Ingot"), 1}}, INT_MAX}
      }));

    // gearPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "gearPress", "center", "ffd", Actions::down, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Iron Gear"), 8}}, {{filterLabel("Iron Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Gold Gear"), 8}}, {{filterLabel("Gold Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Invar Gear"), 8}}, {{filterLabel("Invar Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Copper Gear"), 8}}, {{filterLabel("Copper Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Bronze Gear"), 8}}, {{filterLabel("Bronze Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Nickel Gear"), 8}}, {{filterLabel("Nickel Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Silver Gear"), 8}}, {{filterLabel("Silver Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Lumium Gear"), 8}}, {{filterLabel("Lumium Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Electrum Gear"), 8}}, {{filterLabel("Electrum Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Signalum Gear"), 8}}, {{filterLabel("Signalum Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Enderium Gear"), 8}}, {{filterLabel("Enderium Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Constantan Gear"), 8}}, {{filterLabel("Constantan Ingot"), 4}}, INT_MAX}
      }));

    // wirePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "wirePress", "center", "777", Actions::up, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Wire"), 16}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Wire"), 16}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX}
      }));

    // rodPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "rodPress", "south", "00a", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Ardite Tool Rod"), 16}}, {{filterLabel("Ardite Ingot"), 1}}, INT_MAX}
      }));

    // planter
    factory.addProcess(std::make_unique<ProcessScatteringWorkingSet>(factory, "planter", "center", "7f4", Actions::up, Actions::down,
      4, std::vector<size_t>{6, 7, 8, 9, 11, 12, 13, 14}, nullptr, std::vector<Recipe<>>{
        {{{filterLabel("Wheat"), 64}, {filterLabel("Seeds"), 64}}, {{filterLabel("Seeds"), 1, {}, true}}},
        {{{filterLabel("Sweet Potato"), 64}}, {{filterLabel("Sweet Potato"), 1, {}, true}}},
        {{{filterLabel("Cranberry"), 64}}, {{filterLabel("Cranberry"), 1, {}, true}}},
        {{{filterLabel("Soybean"), 64}}, {{filterLabel("Soybean"), 1, {}, true}}},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Birch Sapling"), 1}}},
        {{{filterLabel("Potato"), 64}}, {{filterLabel("Potato"), 1, {}, true}}},
        {{{filterLabel("Onion"), 64}}, {{filterLabel("Onion"), 1, {}, true}}},
        {{{filterLabel("Flax"), 64}}, {{filterLabel("Flax"), 1, {}, true}}},
        {{{filterLabel("Corn"), 64}}, {{filterLabel("Corn"), 1, {}, true}}}
      }));

    // crusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "crusher", "center", "127", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Bio Fuel"), 16}}, {{filterLabel("Wheat"), 1, {0}}}, 16}
      }));

    // pinkSlime
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pinkSlime", "center", "a96", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Pink Slime"), 16}}, {{filterLabel("Green Slime Block"), 1, {0}}}, 16}
      }));

    // sawMill
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "sawMill", "center", "127", Actions::west, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Birch Wood Planks"), 16}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16},
        {{{filterLabel("Stick"), 16}}, {{filterLabel("Birch Wood Planks"), 1, {0}}}, 16},
        {{{filterLabel("Coal"), 16}}, {{filterLabel("Torch"), 4, {0}}}, 16}
      }));

    // manufactory
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "manufactory", "center", "7b7", Actions::north, Actions::up,
      std::vector<StockEntry>{}, 64, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Niter"), 16}}, {{filterLabel("Sandstone"), 1}}, INT_MAX},
        {{{filterLabel("Clay Dust"), 16}}, {{filterName("minecraft:clay"), 1}}, INT_MAX},
        {{{filterLabel("Boron Dust"), 16}}, {{filterLabel("Boron Ore"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Dust"), 16}}, {{filterLabel("Osmium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Grit"), 16}}, {{filterLabel("Uranium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Lithium Dust"), 16}}, {{filterLabel("Lithium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Thorium Dust"), 16}}, {{filterLabel("Thorium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Dust"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Silicon Ingot"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Magnesium Dust"), 16}}, {{filterLabel("Magnesium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Ore Dust"), 16}}, {{filterLabel("Cobalt Ore"), 1}}, INT_MAX},
        {{{filterLabel("Ardite Ore Dust"), 16}}, {{filterLabel("Ardite Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Tin"), 16}}, {{filterLabel("Tin Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Coal"), 16}}, {{filterLabel("Coal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Iron"), 16}}, {{filterLabel("Iron Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Gold"), 16}}, {{filterLabel("Gold Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Lead"), 16}}, {{filterLabel("Lead Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Copper"), 16}}, {{filterLabel("Copper Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Silver"), 16}}, {{filterLabel("Silver Ore"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Dust"), 16}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Charcoal"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Aluminum"), 16}}, {{filterLabel("Aluminum Ore"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Platinum"), 16}}, {{filterLabel("Platinum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Obsidian"), 16}}, {{filterLabel("Obsidian"), 1}}, INT_MAX},
        {{{filterLabelName("Flour", "appliedenergistics2:material"), 16}}, {{filterLabel("Wheat"), 1}}, INT_MAX}
      }));

    // diamondSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondSieve", "center", "7a5", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Certus Quartz Crystal"), 16}
        }, {{filterLabel("Compressed Dust"), 1, {0}}}, 16},
        {{
          {filterLabel("Aquamarine"), 16}
        }, {{filterLabel("Compressed Sand"), 1, {0}}}, 16},
        {{
          {filterLabel("Nether Quartz"), 16}
        }, {{filterLabel("Compressed Soul Sand"), 1, {0}}}, 16},
        {{
          {filterLabel("Diamond"), 16},
          {filterLabel("Tin Ore Piece"), 16},
          {filterLabel("Iron Ore Piece"), 16},
          {filterLabel("Gold Ore Piece"), 16},
          {filterLabel("Lead Ore Piece"), 16},
          {filterLabel("Nickel Ore Piece"), 16},
          {filterLabel("Copper Ore Piece"), 16},
          {filterLabel("Silver Ore Piece"), 16},
          {filterLabel("Osmium Ore Piece"), 16},
          {filterLabel("Aluminium Ore Piece"), 16},
          {filterLabel("Crushed Black Quartz"), 16}
        }, {{filterLabel("Compressed Gravel"), 1, {0}}}, 16},
        {{
          {filterLabel("Magnesium Ore Piece"), 16},
          {filterLabel("Lithium Ore Piece"), 16},
          {filterLabel("Thorium Ore Piece"), 16},
          {filterLabel("Cobalt Ore Piece"), 16},
          {filterLabel("Ardite Ore Piece"), 16},
          {filterLabel("Boron Ore Piece"), 16}
        }, {{filterLabel("Compressed Nether Gravel"), 1, {0}}}, 16},
        {{
          {filterLabel("Ruby"), 16},
          {filterLabel("Sapphire"), 16},
          {filterLabel("Peridot"), 16},
          {filterLabel("Tanzanite"), 16},
          {filterLabel("Topaz"), 16},
          {filterLabel("Amber"), 16},
          {filterLabel("Malachite"), 16}
        }, {{filterLabel("Compressed Ender Gravel"), 1, {0}}}, 16}
      }));

    // ironSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ironSieve", "south", "737", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Uranium Ore Piece"), 16}
        }, {{filterLabel("Compressed Ender Gravel"), 1, {0}}}, 16}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "furnace", "center", "7a5", Actions::north, Actions::down,
      std::vector<StockEntry>{}, 64, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:netherbrick"), 16}}, {{filterLabel("Netherrack"), 1}}, INT_MAX},
        {{{filterLabel("Brick"), 16}}, {{filterName("minecraft:clay_ball"), 1}}, INT_MAX},
        {{{filterLabel("Glass"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Stone"), 16}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Bread"), 16}}, {{filterLabelName("Flour", "appliedenergistics2:material"), 1}}, INT_MAX},
        {{{filterLabel("Plastic"), 16}}, {{filterLabel("Dry Rubber"), 1}}, INT_MAX},
        {{{filterLabel("Charcoal"), 16}}, {{filterLabel("Birch Wood"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Ingot"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Tin Ingot"), 16}}, {{filterLabel("Pulverized Tin"), 1}}, INT_MAX},
        {{{filterLabel("Terracotta"), 16}}, {{filterName("minecraft:clay"), 1}}, INT_MAX},
        {{{filterLabel("Iron Ingot"), 16}}, {{filterLabel("Pulverized Iron"), 1}}, INT_MAX},
        {{{filterLabel("Gold Ingot"), 16}}, {{filterLabel("Pulverized Gold"), 1}}, INT_MAX},
        {{{filterLabel("Lead Ingot"), 16}}, {{filterLabel("Pulverized Lead"), 1}}, INT_MAX},
        {{{filterLabel("Boron Ingot"), 16}}, {{filterLabel("Boron Dust"), 1}}, INT_MAX},
        {{{filterLabel("Baked Potato"), 16}}, {{filterLabel("Potato"), 1}}, INT_MAX},
        {{{filterLabel("Cactus Green"), 16}}, {{filterLabel("Cactus"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Ingot"), 16}}, {{filterLabel("Osmium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Copper Ingot"), 16}}, {{filterLabel("Pulverized Copper"), 1}}, INT_MAX},
        {{{filterLabel("Silver Ingot"), 16}}, {{filterLabel("Pulverized Silver"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Ingot"), 16}}, {{filterLabel("Cobalt Ore Dust"), 1}}, INT_MAX},
        {{{filterLabel("Ardite Ingot"), 16}}, {{filterLabel("Ardite Ore Dust"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Ingot"), 16}}, {{filterLabel("Uranium Grit"), 1}}, INT_MAX},
        {{{filterLabel("Lithium Ingot"), 16}}, {{filterLabel("Lithium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Thorium Ingot"), 16}}, {{filterLabel("Thorium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Ingot"), 16}}, {{filterLabel("Pulverized Aluminum"), 1}}, INT_MAX},
        {{{filterLabel("Magnesium Ingot"), 16}}, {{filterLabel("Magnesium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Cooked Tofurkey"), 16}}, {{filterLabel("Raw Tofurkey"), 1}}, INT_MAX},
        {{{filterLabel("Printed Circuit Board (PCB)"), 8}}, {{filterLabel("Raw Circuit Board"), 1}}, INT_MAX}
      }));

    // impregnatedStick
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "impregnatedStick", "center", "7b7", Actions::down, Actions::up,
      std::vector<size_t>{12}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Impregnated Stick"), 16}}, {{filterLabel("Birch Wood"), 2, {12}}}, 16}
      }));

    // hardenedCasing
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "hardenedCasing", "center", "a96", Actions::south, Actions::down,
      std::vector<size_t>{12, 13}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Hardened Casing"), 1}}, {
          {filterLabel("Sturdy Casing"), 1, {12}},
          {filterLabel("Diamond"), 4, {13}}
        }, 16}
      }));

    // tinElectronTube
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "tinElectronTube", "center", "1dc", Actions::north, Actions::south,
      std::vector<size_t>{12, 13}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Tin Electron Tube"), 16}}, {
          {filterLabel("Tin Ingot"), 5, {12}},
          {filterLabel("Redstone"), 2, {13}}
        }, 20}
      }));

    // teFrame
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "teFrame", "south", "737", Actions::down, Actions::up,
      std::vector<size_t>{12, 13, 14, 15, 16, 17}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabelName("Machine Frame", "thermalexpansion:frame"), 1}}, {
          {filterLabel("Enori Crystal"), 4, {12}},
          {filterName("rftools:machine_frame"), 1, {13}},
          {filterLabel("Heavy Engineering Block"), 1, {14}},
          {filterLabel("Iron Casing"), 1, {15}},
          {filterLabel("Machine Case"), 1, {17}},
          {filterLabel("Device Frame"), 1, {16}}
        }, 16}
      }));

    // infusedWood
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "infusedWood", "center", "7b7", Actions::south, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Infused Wood"), 16}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "center", "777", Actions::west, Actions::north,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Steel Ingot"), 16}}, {
          {filterLabel("Pulverized Iron"), 1, {0}},
          {filterLabel("Graphite Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Bronze Ingot"), 16}}, {
          {filterLabel("Pulverized Copper"), 3, {0}},
          {filterLabel("Pulverized Tin"), 1, {1}}
        }, 15},
        {{{filterLabel("Invar Ingot"), 16}}, {
          {filterLabel("Pulverized Iron"), 2, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Electrum Ingot"), 16}}, {
          {filterLabel("Pulverized Gold"), 1, {0}},
          {filterLabel("Pulverized Silver"), 1, {1}}
        }, 16},
        {{{filterLabel("Constantan Ingot"), 16}}, {
          {filterLabel("Pulverized Copper"), 1, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Redstone Alloy Ingot"), 16}}, {
          {filterLabel("Redstone"), 1, {0}},
          {filterLabel("Silicon Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Fused Quartz"), 16}}, {
          {filterLabel("Nether Quartz"), 4, {0}},
          {filterLabel("Block of Quartz"), 1, {1}}
        }, 16},
        {{{filterLabel("Energetic Alloy Ingot"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Energetic Blend"), 2, {1}}
        }, 16},
        {{{filterLabel("Vibrant Alloy Ingot"), 16}}, {
          {filterLabel("Energetic Alloy Ingot"), 1, {0}},
          {filterLabel("Ender Pearl"), 1, {1}}
        }, 16},
        {{{filterLabel("Electrical Steel Ingot"), 16}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Silicon Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Shibuichi Alloy"), 16}}, {
          {filterLabel("Pulverized Copper"), 3, {0}},
          {filterLabel("Pulverized Silver"), 1, {1}}
        }, 15},
        {{{filterLabel("Lead Platinum Alloy"), 16}}, {
          {filterLabel("Pulverized Lead"), 3, {0}},
          {filterLabel("Platinum Ingot"), 1, {1}}
        }, 15},
        {{{filterLabel("Tin Silver Alloy"), 16}}, {
          {filterLabel("Pulverized Tin"), 3, {0}},
          {filterLabel("Pulverized Silver"), 1, {1}}
        }, 15},
        {{{filterLabel("Pulsating Iron Ingot"), 16}}, {
          {filterLabel("Pulverized Iron"), 1, {0}},
          {filterLabel("Ender Pearl"), 1, {1}}
        }, 16},
        {{{filterLabel("Ferroboron Alloy"), 16}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Boron Dust"), 1, {1}}
        }, 16},
        {{{filterLabel("Tough Alloy"), 16}}, {
          {filterLabel("Ferroboron Alloy"), 1, {0}},
          {filterLabel("Lithium Dust"), 1, {1}}
        }, 16},
        {{{filterLabel("Hard Carbon Alloy"), 16}}, {
          {filterLabel("Graphite Ingot"), 2, {0}},
          {filterLabel("Diamond"), 1, {1}}
        }, 16},
        {{{filterLabel("Magnesium Diboride Alloy"), 16}}, {
          {filterLabel("Magnesium Dust"), 1, {0}},
          {filterLabel("Boron Dust"), 2, {1}}
        }, 16},
        {{{filterLabel("Manyullyn Ingot"), 16}}, {
          {filterLabel("Ardite Ore Dust"), 1, {0}},
          {filterLabel("Cobalt Ore Dust"), 1, {1}}
        }, 16}
      }));

    // compressedHammer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressedHammer", "center", "312", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Crushed Netherrack"), 16}}, {{filterLabel("Compressed Netherrack"), 1, {0}}}, 16},
        {{{filterLabel("Crushed Endstone"), 16}}, {{filterLabel("Compressed End Stone"), 1, {0}}}, 16},
        {{{filterLabel("Gravel"), 16}}, {{filterLabel("Compressed Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Sand"), 16}}, {{filterLabel("Compressed Gravel"), 1, {0}}}, 16},
        {{{filterLabel("Dust"), 16}}, {{filterLabel("Compressed Sand"), 1, {0}}}, 16}
      }));

    // compressor
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressor", "center", "a96", Actions::north, Actions::down,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Dense Lapis Lazuli Plate"), 8}}, {{filterLabel("Lapis Lazuli Plate"), 9, {6}}}, 18},
        {{{filterLabel("Lapis Lazuli Plate"), 16}}, {{filterLabel("Lapis Lazuli Dust"), 1, {6}}}, 16},
        {{{filterLabel("Energy Crystal"), 8}}, {{filterLabel("Energium Dust"), 9, {6}}}, 18},
        {{{filterLabel("Advanced Alloy"), 8}}, {{filterLabel("Mixed Metal Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Carbon Plate"), 8}}, {{filterLabel("Raw Carbon Mesh"), 1, {6}}}, 16},
        {{{filterLabel("Blaze Rod"), 16}}, {{filterLabel("Blaze Powder"), 5, {6}}}, 20}
      }));

    // extractor
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "extractor", "center", "539", Actions::west, Actions::up,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sulfur"), 16}}, {{filterLabel("Gunpowder"), 1, {6}, true}}, 16}
      }));

    // extruder
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "extruder", "center", "539", Actions::north, Actions::up,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Copper Cable"), 16}}, {{filterLabel("Copper Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Gold Cable"), 16}}, {{filterLabel("Gold Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Tin Cable"), 16}}, {{filterLabel("Tin Ingot"), 1, {6}}}, 16}
      }));

    // roller
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "roller", "center", "a96", Actions::west, Actions::down,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Copper Item Casing"), 16}}, {{filterLabel("Copper Plate"), 1, {6}}}, 16},
        {{{filterLabel("Iron Item Casing"), 16}}, {{filterLabel("Iron Plate"), 1, {6}}}, 16},
        {{{filterLabel("Lead Item Casing"), 16}}, {{filterLabel("Lead Plate"), 1, {6}}}, 16},
        {{{filterLabel("Tin Item Casing"), 16}}, {{filterLabel("Tin Plate"), 1, {6}}}, 16}
      }));

    // treatedWood
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "treatedWood", "center", "312", Actions::west, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 5, 6, 7, 8}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Treated Wood Planks"), 16}}, {{filterLabel("Birch Wood Planks"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}}, 8}
      }));

    // herbaCrystal
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "herbaCrystal", "center", "312", Actions::north, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Herba Vis Crystal"), 16}}, {{filterLabel("Quartz Sliver"), 1, {0}}}, 16}
      }));

    // inductionSmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "inductionSmelter", "center", "539", Actions::south, Actions::up,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rich Slag"), 16}}, {
          {filterLabel("Clock"), 1, {0}},
          {filterLabel("Sand"), 1, {1}}
        }, 16},
        {{
          {filterLabel("Nickel Ingot"), 16},
          {filterLabel("Platinum Ingot"), 16}
        }, {
          {filterLabel("Nickel Ore"), 1, {0}},
          {filterLabel("Cinnabar"), 1, {1}}
        }, 16},
        {{{filterLabel("Iridium Ingot"), 16}}, {
          {filterLabel("Platinum Ore"), 1, {0}},
          {filterLabel("Cinnabar"), 1, {1}}
        }, 16},
        {{{filterLabel("Black Iron Ingot"), 16}}, {
          {filterLabel("Block of Invar"), 1, {0}},
          {filterLabel("Tough Alloy"), 1, {1}}
        }, 16},
        {{{filterLabel("Signalum Cell Frame (Full)"), 1}}, {
          {filterLabel("Signalum Cell Frame (Empty)"), 1, {0}},
          {filterLabel("Block of Redstone"), 40, {1}}
        }, 40}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "ffd", Actions::north, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 16}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // pulverizer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pulverizer", "center", "ffd", Actions::west, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterName("harvestcraft:flouritem"), 16}}, {{filterLabel("Wheat"), 1, {0}}}, 16},
        {{{filterLabel("Dandelion Yellow"), 16}}, {{filterLabel("Dandelion"), 1, {0}}}, 16},
        {{{filterLabel("Ground Cinnamon"), 16}}, {{filterLabel("Cinnamon"), 1, {0}}}, 16},
        {{{filterLabel("Cinnabar"), 16}}, {{filterLabel("Redstone Ore"), 1, {0}}}, 16},
      }));

    // centrifuge
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "centrifuge", "center", "1dc", Actions::west, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sugar"), 16}}, {{filterLabel("Sugar Canes"), 1, {0}}}, 16},
        {{{filterLabel("Cooking Oil"), 16}}, {{filterLabel("Pumpkin"), 1, {0}}}, 16},
        {{{filterLabel("Silken Tofu"), 16}}, {{filterLabel("Soybean"), 1, {0}}}, 16},
        {{
          {filterLabel("Firm Tofu"), 16},
          {filterLabel("Soy Milk"), 16}
        }, {{filterLabel("Silken Tofu"), 1, {0}}}, 16}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "539", Actions::down, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§eInferium Essence"), 36}}, {{filterName("mysticalagriculture:tier4_inferium_seeds"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Essence"), 16}}, {{filterLabel("Lapis Lazuli Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Fiery Ingot Essence"), 16}}, {{filterLabel("Fiery Ingot Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Experience Essence"), 16}}, {{filterLabel("Experience Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nether Wart"), 16}}, {{filterLabel("Nether Wart"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Sugar Canes"), 16}}, {{filterLabel("Sugar Canes"), 1, {}, true}}, INT_MAX},
        {{{filterName("minecraft:melon_block"), 16}}, {{filterLabel("Melon Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 16}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Dandelion"), 16}}, {{filterLabel("Dandelion"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Basalt Essence"), 16}}, {{filterLabel("Basalt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Chorus Fruit"), 16}}, {{filterLabel("Chorus Flower"), 1}}, INT_MAX},
        {{{filterLabel("Blaze Powder"), 16}}, {{filterLabel("Cinderpearl"), 1}}, INT_MAX},
        {{{filterLabel("Quicksilver"), 16}}, {{filterLabel("Shimmerleaf"), 1}}, INT_MAX},
        {{{filterLabel("Ender Pearl"), 16}}, {{filterLabel("Ender Lilly"), 1}}, INT_MAX},
        {{{filterLabel("Cactus"), 16}}, {{filterLabel("Cactus"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Dye Essence"), 16}}, {{filterLabel("Dye Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Pumpkin"), 16}}, {{filterLabel("Pumpkin Seeds"), 1}}, INT_MAX}
      }));

    // cinnamon
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cinnamon", "center", "80c", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Cinnamon"), 16}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // enrichment
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enrichment", "center", "0ed", Actions::down, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Compressed Redstone"), 1}}, {{filterLabel("Redstone"), 1, {0}}}, 16},
        {{{filterLabel("Compressed Diamond"), 1}}, {{filterLabel("Diamond"), 1, {0}}}, 16},
        {{{filterLabel("Compressed Obsidian"), 1}}, {{filterLabel("Refined Obsidian Dust"), 1, {0}}}, 16},
        {{{filterLabel("Black Quartz"), 16}}, {{filterLabel("Crushed Black Quartz"), 1, {0}}}, 16}
      }));

    // redstoneInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneInfuser", "center", "0ed", Actions::south, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enriched Alloy"), 16}}, {{filterLabel("Iron Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Energy Cell Frame"), 1}}, {{filterLabelName("Machine Frame", "thermalexpansion:frame"), 1, {0}}}, 16},
        {{{filterLabel("Basic Control Circuit"), 16}}, {{filterLabel("Osmium Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Reception Coil"), 8}}, {{filterLabel("Gold Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Conductance Coil"), 8}}, {{filterLabel("Electrum Ingot"), 1, {0}}}, 16}
      }));

    // diamondInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondInfuser", "center", "777", Actions::down, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Reinforced Alloy"), 16}}, {{filterLabel("Enriched Alloy"), 1, {0}}}, 16},
        {{{filterLabel("Refined Obsidian Dust"), 16}}, {{filterLabel("Pulverized Obsidian"), 1, {0}}}, 16}
      }));

    // obsidianInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "obsidianInfuser", "south", "547", Actions::east, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Atomic Alloy"), 16}}, {{filterLabel("Reinforced Alloy"), 1, {0}}}, 16}
      }));

    // speedPotion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "speedPotion", "south", "06e", Actions::west, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Potion of Swiftness"), 1}}, {{filterLabel("Glass Bottle"), 1, {0}}}, 16}
      }));

    // healthPotion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "healthPotion", "south", "5a8", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Potion of Healing"), 1}}, {{filterLabel("Glass Bottle"), 1, {0}}}, 16}
      }));

    // miningPotion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "miningPotion", "south", "5a8", Actions::west, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Potion of Haste"), 1}}, {{filterLabel("Glass Bottle"), 1, {0}}}, 16}
      }));

    // combiner
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "combiner", "south", "06e", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Redstone Ore"), 8}}, {{filterLabel("Redstone"), 16, {0}}}, 64},
        {{{filterLabel("Platinum Ore"), 8}}, {{filterLabel("Pulverized Platinum"), 8, {0}}}, 64}
      }));

    // pressurizer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pressurizer", "south", "547", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Silicon Dioxide"), 16}}, {{filterLabel("Clay Dust"), 16, {0}}}, 16},
        {{{filterLabel("Fluorite"), 16}}, {{filterLabel("Crushed Fluorite"), 16, {0}}}, 16}
      }));

    // crafter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafter", "center", "777", Actions::south, Actions::north,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Cake"), 1}}, {
          {filterLabel("Soy Milk"), 3},
          {filterLabel("Sugar"), 2},
          {filterLabel("Raw Tofeeg"), 1},
          {filterName("harvestcraft:flouritem"), 3}
        }, 189}
      }));

    // alchemy
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "alchemy", "south", "737", Actions::south, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:slime_ball"), 16}}, {{filterLabel("Cactus"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Dust"), 16}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Mana Diamond"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX}
      }));

    // conjuration
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "conjuration", "south", "fff", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Netherrack"), 64}}, {{filterLabel("Netherrack"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Redstone"), 32}}, {{filterLabel("Redstone"), 1, {}, true}}, INT_MAX}
      }));

    // starlightTransmutation
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "starlightTransmutation", "south", "fff", Actions::west, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("End Stone"), 16}}, {{filterLabel("Sandstone"), 1, {}, true}}, INT_MAX}
      }));

    // redstoneFluidInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneFluidInfuser", "south", "cb8", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Signalum Ingot"), 16}}, {{filterLabel("Shibuichi Alloy"), 1, {0}}}, 16},
        {{{filterLabel("Fluxed Electrum Ingot"), 16}}, {{filterLabel("Electrum Ingot"), 1, {0}}}, 16}
      }));

    // redstoneFluidTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneFluidTransposer", "south", "00a", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Flux Crystal"), 16}}, {{filterLabel("Diamond"), 1, {0}}}, 16}
      }));

    // enderium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enderium", "south", "cb8", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enderium Ingot"), 16}}, {{filterLabel("Lead Platinum Alloy"), 1, {0}}}, 16}
      }));

    // lumium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "lumium", "south", "cb8", Actions::south, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Lumium Ingot"), 16}}, {{filterLabel("Tin Silver Alloy"), 1, {0}}}, 16}
      }));

    // xpTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "xpTransposer", "south", "06e", Actions::south, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Blizz Powder"), 16}}, {{filterLabel("Snowball"), 2, {0}}}, 16},
        {{{filterLabel("Basalz Powder"), 16}}, {{filterLabel("Pulverized Obsidian"), 2, {0}}}, 16}
      }));

    // reinforcedCellFrame
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "reinforcedCellFrame", "south", "00a", Actions::south, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Reinforced Cell Frame (Full)"), 1}}, {{filterLabel("Reinforced Cell Frame (Empty)"), 1, {0}}}, 16}
      }));

    // rockCrusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCrusher", "south", "547", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Zirconium Dust"), 16},
          {filterLabel("Crushed Fluorite"), 16}
        }, {{filterLabel("Diorite"), 1, {0}}}, 16}
      }));

    // cokeOven
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "cokeOven", "center", "990", Actions::west,
      ProcessRedstoneEmitter::makeNeeded(factory, "cokeOven", filterLabel("Coal Coke"), 16)));

    // blueSlime
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "blueSlime", "south", "11c", Actions::south,
      ProcessRedstoneEmitter::makeNeeded(factory, "blueSlime", blueSlime, 16)));

    // empowerer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "empowerer", "south", "00a", Actions::east, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 4}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Ultimate Control Circuit"), 16}}, {
          {filterLabel("Elite Control Circuit"), 1, {0}},
          {filterLabel("Atomic Alloy"), 4, {1}}
        }, 16},
        {{{filterLabel("Empowered Restonia Crystal Block"), 16}}, {
          {filterLabel("Restonia Crystal Block"), 1, {0}},
          {filterLabel("Rhodochrosite"), 1, {1}},
          {filterLabel("Ardite Tool Rod"), 1, {2}},
          {filterLabel("Red Nether Brick"), 1, {3}},
          {filterLabel("Redstone Reception Coil"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Void Crystal Block"), 16}}, {
          {filterLabel("Void Crystal Block"), 1, {0}},
          {filterLabel("Basalt"), 1, {1}},
          {filterLabel("Block of Black Quartz"), 1, {2}},
          {filterLabel("Block of Black Iron"), 1, {3}},
          {filterLabel("Ink Sac"), 1, {4}}
        }, 4},
        {{{filterLabel("Hardened Cell Frame"), 1}}, {
          {filterLabel("Energy Cell Frame"), 1, {0}},
          {filterLabel("Invar Plate"), 1, {1}},
          {filterLabel("Steel Rod"), 1, {2}},
          {filterLabel("Steel Casing"), 1, {3}},
          {filterLabel("Invar Gear"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Palis Crystal Block"), 1}}, {
          {filterLabel("Palis Crystal Block"), 1, {0}},
          {filterLabel("Dense Lapis Lazuli Plate"), 1, {1}},
          {filterLabel("Cobalt Ingot"), 1, {2}},
          {filterLabel("Sapphire"), 1, {3}},
          {filterLabel("Congealed Slime Block"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Diamatine Crystal Block"), 1}}, {
          {filterLabel("Diamatine Crystal Block"), 1, {0}},
          {filterLabel("Mana Diamond"), 1, {1}},
          {filterLabel("Malachite"), 1, {2}},
          {filterLabel("Manyullyn Ingot"), 1, {3}},
          {filterLabel("Zirconium Dust"), 1, {4}}
        }, 4}
      }));

    // reactor
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "center"));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
