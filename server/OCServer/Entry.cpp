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

    auto blueSlime(filterFn([](const Item &item) {
      return item.name == "tconstruct:edible" && item.damage == 1;
    }));

    auto congealedBlueSlime(filterFn([](const Item &item) {
      return item.name == "tconstruct:slime_congealed" && item.damage == 1;
    }));

    auto crystaltineTrimmed(filterFn([](const Item &item) {
      return item.name == "extendedcrafting:trimmed" && item.damage == 4;
    }));

    Factory factory(server, 1000, "south", {
      {"center", "127", Actions::up},
      {"south", "fff", Actions::down},
      {"west", "cfa", Actions::west},
      {"east", "1d7", Actions::up}
    });
    factory.addStorage(std::make_unique<StorageChest>(factory, "center", "7f4", Actions::south, Actions::down));
    factory.addStorage(std::make_unique<StorageChest>(factory, "south", "fff", Actions::up, Actions::down));
    factory.addStorage(std::make_unique<StorageChest>(factory, "west", "cfa", Actions::up, Actions::west));
    factory.addStorage(std::make_unique<StorageChest>(factory, "west", "dfc", Actions::south, Actions::west));
    factory.addStorage(std::make_unique<StorageChest>(factory, "west", "10b", Actions::north, Actions::south));
    factory.addStorage(std::make_unique<StorageChest>(factory, "east", "1d7", Actions::south, Actions::up));
    factory.addStorage(std::make_unique<StorageDrawer>(factory, "center", "127", Actions::down, Actions::up, std::vector<SharedItemFilter>{
      filterLabel("Birch Sapling"), filterLabel("Experience Seeds"), filterLabel("Seeds"), filterLabel("Wheat"),
      filterName("mysticalagriculture:tier4_inferium_seeds"), filterLabel("Substrate"), filterLabel("Mysterious Comb"), filterLabel("Mana Infused Ore"),
      filterLabel("Litherite Crystal"), filterLabel("Sawdust"), filterLabel("Redstone Seeds"), filterLabel("Iron Ore"),
      filterLabel("Magnesium Ore"), filterLabel("Cobalt Ore"), filterLabel("Lapis Lazuli"), filterLabel("Copper Ore"),
      filterLabel("Platinum Ore"), filterLabel("Pulverized Coal"), filterLabel("Lead Ore"), filterLabel("Ardite Ore"),
      filterLabel("Veggie Bait"), filterLabel("Silver Ore"), filterLabel("Nickel Ore"), filterLabel("Aluminum Ore"),
      filterLabel("Pumpkin Seeds"), filterLabel("Grain Bait"), filterLabel("Cobblestone"), filterLabel("Gold Ore"),
      filterLabel("Coal Coke"), filterLabel("Lapis Lazuli Seeds"), filterLabel("Grains of Infinity"), filterLabel("Thorium Ore"),
      filterLabel("Tiny Pile of Plutonium"), filterLabel("Iridium Ore"), filterLabel("Nether Quartz"), filterLabel("Tin Ore"),
      filterLabel("Diamond"), filterLabel("Emerald"), filterLabel("Glowstone"), filterLabel("Redstone"),
      filterLabel("Aethium Crystal"), filterLabel("Cinnabar"), filterLabel("Uranium-238"), filterLabel("Fresh Water"),
      filterLabel("Draconium Ore"), filterLabel("Uranium Seeds"), filterLabel("Ionite Crystal"), filterLabel("Cyanite Ingot"),
      filterLabel("Dimensional Shard"), filterLabel("Erodium Crystal"), filterLabel("Uranium 238"), filterLabel("Nether Star"),
      filterLabel("Kyronite Crystal"), filterLabel("Coal"), filterLabel("Bone"), filterLabel("Pladium Crystal"),
      filterLabel("Firm Tofu"), filterLabel("Osmium Ore"), filterLabel("Wither Ash"), filterLabel("Aquamarine"),
      filterLabel("Witherbone"), filterLabel("Necrotic Bone"), filterLabel("Black Quartz"), filterLabel("Apatite"),
      filterLabel("Iron Seeds"), filterLabel("Soybean Seed"), filterLabel("Onion Seed"), filterLabel("Dragon Egg"),
      filterLabel("Platinum Ingot"), filterLabel("Crushed Carobbiite"), filterLabel("Charged Certus Quartz Crystal"), filterLabel("Certus Quartz Crystal"),
      filterLabel("Dry Rubber"), filterLabel("Prosperity Shard"), filterLabel("Uranium Ore"), filterLabel("Sulfur"),
      filterLabel("Singularity")
    }));

    factory.addBackup(filterName("minecraft:brown_mushroom"), 32);
    factory.addBackup(filterLabel("Ebony Substance"), 32);
    factory.addBackup(filterLabel("Ivory Substance"), 32);
    factory.addBackup(filterLabel("Psimetal Ingot"), 32);
    factory.addBackup(filterLabel("Cocoa Beans"), 32);
    factory.addBackup(filterLabel("Sugar Canes"), 32);
    factory.addBackup(filterLabel("Nether Wart"), 32);
    factory.addBackup(filterLabel("Netherrack"), 32);
    factory.addBackup(filterLabel("Dandelion"), 32);
    factory.addBackup(filterLabel("Psidust"), 32);
    factory.addBackup(filterLabel("Psigem"), 32);
    factory.addBackup(filterLabel("Potato"), 32);
    factory.addBackup(filterLabel("Carrot"), 32);
    factory.addBackup(filterLabel("Cactus"), 32);
    factory.addBackup(filterLabel("Seeds"), 32);
    factory.addBackup(filterLabel("Flax"), 32);

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "west", "9a7", Actions::west, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, outAll, std::vector<Recipe<int>>{}));

    // stock
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "stock", "center", "7f4", Actions::west, Actions::down,
      std::vector<StockEntry>{
        {filterLabel("Enriched Uranium Nuclear Fuel"), 64}, // ncFission
        {filterLabel("Titanium Iridium Alloy Ingot"), 64},  // neutronium
        {filterLabel("Compressed Diamond Hammer"), 1},      // compressedHammer
        {filterName("minecraft:brown_mushroom"), 64},       // fungiInfuser
        {filterName("harvestcraft:flouritem"), 64},         // kekimurus
        {filterLabel("Compressed Redstone"), 64},           // redstoneInfuser
        {filterLabel("Compressed Obsidian"), 64},           // obsidianInfuser
        {filterLabel("Compressed Diamond"), 64},            // diamondInfuser
        {filterLabel("Experience Essence"), 64},            // essenceOfKnowledge
        {filterLabel("Lapis Lazuli Dust"), 64},             // coolantCarpenter
        {filterLabel("Birch Wood Planks"), 64},             // furnace
        {filterLabel("Fluxed Phyto-Gro"), 64},              // phyto, netherStar
        {filterLabel("Genetics Labware"), 64},              // dnaCarpenter
        {filterLabel("Dandelion Yellow"), 64},              // printer
        {filterLabel("Osgloglas Ingot"), 64},               // manaCarpenter
        {filterLabel("Vibrant Crystal"), 64},               // enderCrystal
        {filterLabel("Glowstone Dust"), 64},                // energizedGlowstone
        {filterLabel("Cryotheum Dust"), 64},                // gelidCryotheum
        {filterLabel("Uranium Ingot"), 64},                 // reactor
        {filterLabel("Bronze Ingot"), 64},                  // brassIngot
        {filterLabel("Mirion Ingot"), 64},                  // manaCarpenter
        {filterLabel("Lumium Ingot"), 64},                  // reinforcedCellFrame
        {filterLabel("Osmium Ingot"), 64},                  // osmiumCompressor
        {filterLabel("Ender Pearl"), 64},                   // resonantEnder
        {filterLabel("Birch Wood"), 64},                    // latex
        {filterLabel("Raw Tofeeg"), 64},                    // kekimurus
        {filterLabel("Aquamarine"), 64},                    // liquidStarlight
        {filterLabel("Netherrack"), 64},                    // lava
        {filterLabel("Bone Meal"), 64},                     // ironberries
        {filterLabel("Soy Milk"), 64},                      // kekimurus
        {filterLabel("Bio Fuel"), 64},                      // gasTurbine, bioInfuser
        {filterLabel("Charcoal"), 64},                      // terraCrystal, alloying, manaCarpenter, brassIngot
        {filterLabel("Redstone"), 64},                      // destabilizedRedstone
        {filterLabel("Sulfur"), 64},                        // calciumSulfate
        {filterLabel("Paper"), 64},                         // printer
        {filterLabel("Sugar"), 64},                         // kekimurus
        {filterLabel("Seeds"), 64},                         // seedOil
        {filterLabel("Salt"), 192},                         // advancedMetallurgicFabricator
        {filterLabel("Salt"), 192},                         // ^
        {filterLabel("Salt"), 192},                         // ^
        {filterLabel("Sand"), 64},                          // advancedThermionicFabricator, sandInduction
        {filterLabel("Coal"), 64},                          // creosote, cokeOven, coalCarpenter
        {filterLabel("Book"), 64},                          // arcaneEnsorcellator
        {filterLabel("Dirt"), 64}                           // terraCrystal
      }, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{}));

    // obsidianGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "obsidianGen", "south", "06e", Actions::down, Actions::up,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Obsidian"), 128)));

    // snowballGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "snowballGen", "south", "cb8", Actions::east, Actions::down,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Snowball"), 128)));

    // rosinGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "rosinGen", "south", "547", Actions::south, Actions::down,
      1, ProcessInputless::makeNeeded(factory, filterLabel("Rosin"), 128)));

    // autoCompressor
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "autoCompressor", "center", "7b7", Actions::west, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Tiny Dry Rubber"), 9}}, INT_MAX},
        {{}, {{filterLabel("Uranium Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Dragon Egg Essence"), 9}}, INT_MAX},
        {{}, {{filterLabel("Nether Star Essence"), 9}}, INT_MAX},
        {{{filterLabel("Pladium"), 16}}, {{filterLabel("Pladium Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Aethium"), 16}}, {{filterLabel("Aethium Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Redstone"), 256}}, {{filterLabel("Redstone Essence"), 9}}, INT_MAX},
        {{{filterLabel("Bone Block"), 16}}, {{filterLabel("Bone Meal"), 9}}, INT_MAX},
        {{{filterLabel("Blaze Mesh"), 16}}, {{filterLabel("Blaze Rod"), 9}}, INT_MAX},
        {{{filterLabel("Fluix Block"), 16}}, {{filterLabel("Fluix Crystal"), 4}}, INT_MAX},
        {{{filterLabel("Stone Bricks"), 16}}, {{filterLabel("Stone"), 4}}, INT_MAX},
        {{{filterLabel("Osmium Block"), 16}}, {{filterLabel("Osmium Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Mirion Block"), 16}}, {{filterLabel("Mirion Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Tin"), 16}}, {{filterLabel("Tin Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Lead"), 16}}, {{filterLabel("Lead Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Iron"), 16}}, {{filterLabel("Iron Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Cyanite Block"), 16}}, {{filterLabel("Cyanite Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Invar"), 16}}, {{filterLabel("Invar Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Graphite Block"), 16}}, {{filterLabel("Graphite Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Polished Stone"), 16}}, {{filterLabel("Stone Bricks"), 4}}, INT_MAX},
        {{{filterLabel("Crafting Table"), 16}}, {{filterLabel("Birch Wood Planks"), 4}}, INT_MAX},
        {{{filterLabel("Draconium Block"), 16}}, {{filterLabel("Draconium Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Cobalt"), 16}}, {{filterLabel("Cobalt Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Sand"), 16}}, {{filterLabel("Sand"), 9}}, INT_MAX},
        {{{filterLabel("Osgloglas Block"), 16}}, {{filterLabel("Osgloglas Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Enderium"), 16}}, {{filterLabel("Enderium Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Green Slime Block"), 16}}, {{filterName("minecraft:slime_ball"), 9}}, INT_MAX},
        {{{filterLabel("Block of Electrum"), 16}}, {{filterLabel("Electrum Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Redstone"), 40}}, {{filterLabel("Redstone"), 9}}, INT_MAX},
        {{{filterLabel("Void Crystal Block"), 16}}, {{filterLabel("Void Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Block"), 16}}, {{filterLabel("Lapis Lazuli"), 9}}, INT_MAX},
        {{{filterLabel("Palis Crystal Block"), 16}}, {{filterLabel("Palis Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Enori Crystal Block"), 16}}, {{filterLabel("Enori Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Block of Elementium"), 16}}, {{filterLabel("Elementium Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Compressed End Stone"), 16}}, {{filterLabel("End Stone"), 9}}, INT_MAX},
        {{{filterLabel("Block of Demon Metal"), 16}}, {{filterLabel("Demon Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Black Quartz"), 16}}, {{filterLabel("Black Quartz"), 4}}, INT_MAX},
        {{{filterLabel("Compressed Netherrack"), 16}}, {{filterLabel("Netherrack"), 9}}, INT_MAX},
        {{{filterLabel("Restonia Crystal Block"), 16}}, {{filterLabel("Restonia Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Emeradic Crystal Block"), 16}}, {{filterLabel("Emeradic Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Block of Enhanced Ender"), 16}}, {{filterLabel("Enhanced Ender Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Diamatine Crystal Block"), 16}}, {{filterLabel("Diamatine Crystal"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Ender Gravel"), 16}}, {{filterLabel("Crushed Endstone"), 9}}, INT_MAX},
        {{{filterLabel("Stable-'Unstable Ingot'"), 16}}, {{filterLabel("Stable-'Unstable Nugget'"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Nether Gravel"), 16}}, {{filterLabel("Crushed Netherrack"), 9}}, INT_MAX},
        {{{filterLabel("Titanium Aluminide Block"), 16}}, {{filterLabel("Titanium Aluminide Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Compressed Crafting Table"), 16}}, {{filterLabel("Crafting Table"), 9}}, INT_MAX},
        {{{filterLabel("Block of Evil Infused Iron"), 16}}, {{filterLabel("Evil Infused Iron Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Block of Mana Infused Metal"), 16}}, {{filterLabel("Mana Infused Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Titanium Iridium Alloy Block"), 16}}, {{filterLabel("Titanium Iridium Alloy Ingot"), 9}}, INT_MAX},
        {{{filterLabel("Double Compressed Crafting Table"), 16}}, {{filterLabel("Compressed Crafting Table"), 9}}, INT_MAX},
        {{{filterLabelName("Block of Black Iron", "extendedcrafting:storage"), 16}}, {{filterLabel("Black Iron Ingot"), 9}}, INT_MAX}
      }));

    // pureDaisy
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pureDaisy", "center", "7a5", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 8, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Livingwood"), 16}}, {{filterLabel("Infused Wood"), 1}}, INT_MAX},
        {{{filterLabel("Livingrock"), 16}}, {{filterLabel("Arcane Stone"), 1}}, INT_MAX}
      }));

    // advancedMetallurgicFabricator
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "advancedMetallurgicFabricator", "south", "e22", Actions::north, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Titanium Ingot"), 16}}, {
          {filterLabel("Magnesium Ore"), 2},
          {filterLabel("Carbon Plate"), 1},
          {filterLabel("Salt"), 4}
        }, INT_MAX},
        {{{filterLabel("Osgloglas Ingot"), 16}}, {
          {filterLabel("Refined Obsidian Ingot"), 1},
          {filterLabel("Glowstone Ingot"), 1},
          {filterLabel("Osmium Ingot"), 1}
        }, INT_MAX},
        {{{filterLabel("Modularium Alloy"), 16}}, {
          {filterLabel("Empowered Palis Crystal"), 1},
          {filterLabel("Electrical Steel Ingot"), 2},
          {filterLabel("Platinum Ingot"), 1}
        }, INT_MAX},
        {{{filterLabel("Osmiridium Ingot"), 16}}, {
          {filterLabel("Iridium Ingot"), 1},
          {filterLabel("Osmium Ingot"), 1}
        }, INT_MAX}
      }));

    // ironMechanicalComponent
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "ironMechanicalComponent", "center", "1dc", Actions::north, Actions::south,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Iron Mechanical Component"), 16}}, {
          {filterLabel("Iron Plate"), 2},
          {filterLabel("Copper Ingot"), 1}
        }, INT_MAX}
      }));

    // steelMechanicalComponent
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "steelMechanicalComponent", "west", "9a7", Actions::up, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Steel Mechanical Component"), 16}}, {
          {filterLabel("Steel Plate"), 2},
          {filterLabel("Copper Ingot"), 1}
        }, INT_MAX}
      }));

    // vacuumTube
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "vacuumTube", "west", "cfa", Actions::south, Actions::west,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Vacuum Tube"), 16}}, {
          {filterLabel("Glass"), 1},
          {filterLabel("Nickel Plate"), 1},
          {filterLabel("Copper Wire"), 1},
          {filterLabel("Redstone"), 1}
        }, INT_MAX}
      }));

    // circuitBoard
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "circuitBoard", "west", "9a7", Actions::south, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Circuit Board"), 16}}, {
          {filterLabel("Insulating Glass"), 1},
          {filterLabel("Copper Plate"), 1},
          {filterLabel("Vacuum Tube"), 2}
        }, INT_MAX}
      }));

    // waterBarrel
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "waterBarrel", "center", "80c", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:clay"), 16}}, {{filterLabel("Dust"), 1}}, INT_MAX},
        {{{filterLabel("Black Concrete"), 16}}, {{filterLabel("Black Concrete Powder"), 1}}, INT_MAX}
      }));

    // lathe
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "lathe", "west", "cfa", Actions::north, Actions::west,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Rod"), 16}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Titanium Rod"), 16}}, {{filterLabel("Titanium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Steel Rod"), 16}}, {{filterLabel("Steel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Rod"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX}
      }));

    // crystallizer
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crystallizer", "west", "9a7", Actions::north, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Silicon Boule"), 16}}, {{filterLabel("Silicon Ingot"), 1}}, INT_MAX}
      }));

    // cuttingMachine
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "cuttingMachine", "east", "ad9", Actions::north, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Silicon Wafer"), 16}}, {{filterLabel("Silicon Boule"), 1}}, INT_MAX},
        {{{filterLabel("Advanced Circuit"), 16}}, {{filterLabel("Advanced Circuit Plate"), 1}}, INT_MAX}
      }));

    // alloying
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloying", "east", "ad9", Actions::south, Actions::down,
      std::vector<size_t>{0, 1, 2, 3, 4}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Alumite Ingot"), 16}}, {
          {filterLabel("Obsidian"), 1, {0}},
          {filterLabel("Iron Ingot"), 2, {1}},
          {filterLabel("Aluminum Ingot"), 5, {2}}
        }, 15},
        {{{filterLabel("Mirion Ingot"), 16}}, {
          {filterLabel("Glass"), 1, {0}},
          {filterLabel("Terrasteel Ingot"), 1, {1}},
          {filterLabel("Manasteel Ingot"), 1, {2}},
          {filterLabel("Elementium Ingot"), 1, {3}},
          {filterLabel("Cobalt Ingot"), 1, {4}}
        }, 4},
        {{{filterLabel("Pigiron Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 2, {0}},
          {filterLabel("Rotten Flesh"), 2, {1}},
          {filterName("minecraft:clay_ball"), 1, {2}}
        }, 16}
      }));

    // crystaltineIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "crystaltineIngot", "east", "ad9", Actions::up, Actions::down,
      std::vector<size_t>{0, 1, 2, 3, 4}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Crystaltine Ingot"), 16}}, {
          {filterLabel("Diamond"), 8, {0}},
          {filterLabel("Iron Ingot"), 4, {1}},
          {filterLabel("Gold Ingot"), 2, {2}},
          {filterLabel("Lapis Lazuli"), 10, {3}},
          {filterLabel("Nether Star Nugget"), 4, {4}},
        }, INT_MAX}
      }));

    // enderCrystal
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enderCrystal", "west", "dfc", Actions::north, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Ender Crystal"), 16}}, {{filterLabel("Soul Vial"), 1, {0}}}, 16}
      }));

    // enderCrystal
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manganeseDioxide", "west", "dfc", Actions::up, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Manganese Dioxide Dust"), 16}}, {{filterLabel("Manganese Oxide Dust"), 1, {0}}}, 16}
      }));

    // demonIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "demonIngot", "west", "dfc", Actions::east, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{}, {{filterLabel("Stone Sword"), 1, {0}}}, INT_MAX},
        {{{filterLabel("Demon Ingot"), 16}}, {{filterLabel("Gold Ingot"), 1, {0}}}, 16},
      }));

    // advancedCraftingTable
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "advancedCraftingTable", "east", "ad9", Actions::east, Actions::down,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Advanced Crafting Table"), 4}}, {
          {filterLabel("Basic Component"), 1, {0}},
          {filterLabel("Basic Catalyst"), 1, {1}},
          {filterLabel("Advanced Component"), 1, {2}},
          {filterLabel("Advanced Catalyst"), 1, {3}},
          {filterLabel("Elite Component"), 1, {4}},
          {filterLabel("Elite Catalyst"), 1, {5}},
          {filterLabel("Ultimate Component"), 1, {6}},
          {filterLabel("Ultimate Catalyst"), 1, {7}},
          {filterLabel("Basic Crafting Table"), 1, {8}}
        }, 4}
      }));

    // eliteCraftingTable
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "eliteCraftingTable", "east", "caf", Actions::south, Actions::north,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Elite Crafting Table"), 4}}, {
          {filterLabel("Signalum Cell Frame (Full)"), 2, {0}},
          {filterLabel("Advanced Crafting Table"), 2, {1}},
          {filterLabel("Hardened Cell Frame"), 4, {2}},
          {filterLabel("Fluxed Phyto-Gro"), 2, {3}},
          {filterLabel("Elite Component"), 8, {4}},
          {filterLabel("Osgloglas Block"), 2, {5}},
          {filterLabel("Crafter Tier 3"), 1, {6}},
          {filterLabel("Mana Dust"), 4, {7}},
        }, 16}
      }));

    // ultimateCraftingTable
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateCraftingTable", "east", "caf", Actions::west, Actions::north,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Ultimate Crafting Table"), 4}}, {
          {filterLabel("Double Compressed Crafting Table"), 8, {0}},
          {filterLabel("Signalum Cell Frame (Full)"), 4, {1}},
          {filterLabel("Resonant Cell Frame (Full)"), 2, {2}},
          {filterLabel("Iridium Neutron Reflector"), 2, {3}},
          {filterLabel("Advanced Crafting Table"), 4, {4}},
          {filterLabel("Extreme Crafting Table"), 1, {5}},
          {filterLabel("Elite Crafting Table"), 2, {6}},
          {filterLabel("Block of Enderium"), 1, {7}},
          {filterLabel("Draconium Crystal"), 8, {8}},
          {filterLabel("Advanced Circuit"), 4, {9}},
          {filterLabel("Ludicrite Block"), 1, {10}},
          {filterLabel("Mirion Block"), 1, {11}},
          {filterLabel("Wyvern Core"), 2, {12}},
          {filterLabel("Aethium"), 1, {13}},
          {crystaltineTrimmed, 8, {14}}
        }, 16}
      }));

    // ultimateStew
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateStew", "east", "caf", Actions::down, Actions::north,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
        27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Ultimate Stew"), 16}}, {
          {filterName("minecraft:brown_mushroom"), 1, {0}},
          {filterName("minecraft:melon_block"), 1, {1}},
          {filterLabel("Neutronium Ingot"), 1, {2}},
          {filterLabel("Brussel Sprout"), 1, {3}},
          {filterLabel("Water Chestnut"), 1, {4}},
          {filterLabel("Winter Squash"), 1, {5}},
          {filterLabel("Bamboo Shoot"), 1, {6}},
          {filterLabel("Sweet Potato"), 1, {7}},
          {filterLabel("Cauliflower"), 1, {8}},
          {filterLabel("Ironberries"), 1, {9}},
          {filterLabel("Wildberries"), 1, {10}},
          {filterLabel("Bellpepper"), 1, {11}},
          {filterLabel("Spice Leaf"), 1, {12}},
          {filterLabel("Blackberry"), 1, {13}},
          {filterLabel("Strawberry"), 1, {14}},
          {filterLabel("Gooseberry"), 1, {15}},
          {filterLabel("Artichoke"), 1, {16}},
          {filterLabel("Blueberry"), 1, {17}},
          {filterLabel("Raspberry"), 1, {18}},
          {filterLabel("Cranberry"), 1, {19}},
          {filterLabel("Asparagus"), 1, {20}},
          {filterLabel("Beetroot"), 2, {21}},
          {filterLabel("Cucumber"), 1, {22}},
          {filterLabel("Zucchini"), 1, {23}},
          {filterLabel("Rutabaga"), 1, {24}},
          {filterLabel("Broccoli"), 1, {25}},
          {filterLabel("Eggplant"), 1, {26}},
          {filterLabel("Scallion"), 1, {27}},
          {filterLabel("Seaweed"), 1, {28}},
          {filterLabel("Lettuce"), 1, {29}},
          {filterLabel("Cabbage"), 1, {30}},
          {filterLabel("Spinach"), 1, {31}},
          {filterLabel("Parsnip"), 1, {32}},
          {filterLabel("Pumpkin"), 1, {33}},
          {filterLabel("Rhubarb"), 1, {34}},
          {filterLabel("Radish"), 1, {35}},
          {filterLabel("Turnip"), 1, {36}},
          {filterLabel("Celery"), 1, {37}},
          {filterLabel("Carrot"), 1, {38}},
          {filterLabel("Potato"), 1, {39}},
          {filterLabel("Tomato"), 1, {40}},
          {filterLabel("Garlic"), 1, {41}},
          {filterLabel("Grapes"), 1, {42}},
          {filterLabel("Wheat"), 1, {43}},
          {filterLabel("Apple"), 1, {44}},
          {filterLabel("Onion"), 1, {45}},
          {filterLabel("Bean"), 1, {46}},
          {filterLabel("Corn"), 1, {47}},
          {filterLabel("Leek"), 1, {48}},
          {filterLabel("Okra"), 1, {49}},
          {filterLabel("Peas"), 1, {50}}
        }, 2}
      }));

    // cosmicMeatballs
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cosmicMeatballs", "west", "10b", Actions::east, Actions::south,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Cosmic Meatballs"), 16}}, {
          {filterLabel("Raw Prime Chicken"), 1, {0}},
          {filterLabel("Raw Prime Peacock"), 1, {1}},
          {filterLabel("Raw Prime Mutton"), 1, {2}},
          {filterLabel("Raw Prime Rabbit"), 1, {3}},
          {filterLabel("Raw Prime Chevon"), 1, {4}},
          {filterLabel("Neutronium Ingot"), 1, {5}},
          {filterLabel("Raw Prime Steak"), 1, {6}},
          {filterLabel("Raw Prime Bacon"), 1, {7}},
          {filterLabel("Raw Prime Beef"), 1, {8}},
          {filterLabel("Raw Prime Pork"), 1, {9}},
          {filterLabel("Raw Frog Legs"), 1, {10}},
          {filterLabel("Raw Porkchop"), 1, {11}},
          {filterLabel("Raw Chicken"), 1, {12}},
          {filterLabel("Raw Peacock"), 1, {13}},
          {filterLabel("Raw Venison"), 1, {14}},
          {filterLabel("Raw Chevon"), 1, {15}},
          {filterLabel("Raw Rabbit"), 1, {16}},
          {filterLabel("Raw Mutton"), 1, {17}},
          {filterLabel("Raw Turkey"), 1, {18}},
          {filterLabel("Raw Horse"), 1, {19}},
          {filterLabel("Raw Beef"), 1, {20}},
          {filterLabel("Raw Duck"), 1, {21}}
        }, 2}
      }));

    // enderStar
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enderStar", "south", "5a8", Actions::west, Actions::up,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabelName("Ender Star", "extendedcrafting:material"), 16}}, {
          {filterLabel("Nether Star"), 1, {0}},
          {filterLabel("Eye of Ender"), 4, {1}}
        }, 16}
      }));

    // enhancedEnderIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enhancedEnderIngot", "south", "5a8", Actions::down, Actions::up,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enhanced Ender Ingot"), 16}}, {
          {filterLabelName("Ender Star", "extendedcrafting:material"), 1, {0}},
          {filterLabel("Ender Ingot"), 4, {1}}
        }, 16}
      }));

    // precisionAssembler
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "precisionAssembler", "east", "1d7", Actions::north, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Advanced Circuit Plate"), 16}}, {
          {filterLabel("Silicon Wafer"), 1},
          {filterLabel("Redstone Alloy Grinding Ball"), 1},
          {filterLabel("Intricate Circuit Board"), 1}
        }, INT_MAX}
      }));

    // atomicReconstructor
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "atomicReconstructor", "center", "80c", Actions::west, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Ethetic Green Block"), 16}}, {{filterLabel("Chiseled Quartz Block"), 1}}, INT_MAX},
        {{{filterLabel("Diamatine Crystal"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Restonia Crystal"), 16}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Emeradic Crystal"), 16}}, {{filterLabel("Emerald"), 1}}, INT_MAX},
        {{{filterLabel("Enori Crystal"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Palis Crystal"), 16}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
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
        {{{filterLabel("Brass Plate"), 16}}, {{filterLabel("Alchemical Brass Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Nickel Plate"), 16}}, {{filterLabel("Nickel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Plate"), 16}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bronze Plate"), 16}}, {{filterLabel("Bronze Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Plate"), 16}}, {{filterLabel("Uranium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Plate"), 16}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Signalum Plate"), 16}}, {{filterLabel("Signalum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Electrum Plate"), 16}}, {{filterLabel("Electrum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Titanium Plate"), 16}}, {{filterLabel("Titanium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Thaumium Plate"), 16}}, {{filterLabel("Thaumium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Large Plate"), 4}}, {{filterLabel("Block of Iron"), 1}}, INT_MAX},
        {{{filterLabel("Void Metal Plate"), 16}}, {{filterLabel("Void Metal Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Electrum Large Plate"), 4}}, {{filterLabel("Block of Electrum"), 1}}, INT_MAX},
        {{{filterLabel("Fluxed Electrum Plate"), 16}}, {{filterLabel("Fluxed Electrum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Titanium Aluminide Plate"), 16}}, {{filterLabel("Titanium Aluminide Ingot"), 1}}, INT_MAX}
      }));

    // gearPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "gearPress", "center", "ffd", Actions::down, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Lead Gear"), 8}}, {{filterLabel("Lead Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Iron Gear"), 8}}, {{filterLabel("Iron Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Gold Gear"), 8}}, {{filterLabel("Gold Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Invar Gear"), 8}}, {{filterLabel("Invar Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Steel Gear"), 8}}, {{filterLabel("Steel Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Copper Gear"), 8}}, {{filterLabel("Copper Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Bronze Gear"), 8}}, {{filterLabel("Bronze Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Nickel Gear"), 8}}, {{filterLabel("Nickel Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Silver Gear"), 8}}, {{filterLabel("Silver Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Lumium Gear"), 8}}, {{filterLabel("Lumium Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Electrum Gear"), 8}}, {{filterLabel("Electrum Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Signalum Gear"), 8}}, {{filterLabel("Signalum Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Enderium Gear"), 8}}, {{filterLabel("Enderium Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Constantan Gear"), 8}}, {{filterLabel("Constantan Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Dark Bimetal Gear"), 8}}, {{filterLabel("Dark Steel Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Mana Infused Gear"), 8}}, {{filterLabel("Mana Infused Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Vibrant Bimetal Gear"), 8}}, {{filterLabel("Vibrant Alloy Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Energized Bimetal Gear"), 8}}, {{filterLabel("Energetic Alloy Ingot"), 4}}, INT_MAX}
      }));

    // wirePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "wirePress", "center", "0ed", Actions::down, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Wire"), 16}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Wire"), 16}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX}
      }));

    // rodPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "rodPress", "south", "00a", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Ardite Tool Rod"), 16}}, {{filterLabel("Ardite Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iridium Rod"), 16}}, {{filterLabel("Iridium Ingot"), 1}}, INT_MAX}
      }));

    // crusher
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crusher", "center", "127", Actions::east, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Bio Fuel"), 16}}, {{filterLabel("Pumpkin"), 1}}, INT_MAX},
        {{{filterLabel("Coke Dust"), 16}}, {{filterLabel("Coal Coke"), 1}}, INT_MAX},
        {{{filterLabel("Certus Quartz Dust"), 16}}, {{filterLabel("Charged Certus Quartz Crystal"), 1}}, INT_MAX}
      }));

    // pinkSlime
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pinkSlime", "center", "a96", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Pink Slime"), 16}}, {{filterLabel("Green Slime Block"), 1, {0}}}, 16}
      }));

    // sawMill
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sawMill", "center", "127", Actions::west, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Birch Wood Planks"), 64}}, {{filterLabel("Birch Wood"), 1}}, INT_MAX},
        {{{filterLabel("Ironwood Planks"), 64}}, {{filterLabel("Ironwood"), 1}}, INT_MAX},
        {{{filterLabel("Stick"), 16}}, {{filterLabel("Birch Wood Planks"), 1}}, INT_MAX}
      }));

    // manufactory
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "manufactory", "center", "7b7", Actions::north, Actions::up,
      std::vector<StockEntry>{}, 256, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Sand"), 16}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Niter"), 16}}, {{filterLabel("Sandstone"), 1}}, INT_MAX},
        {{{filterLabel("Flint"), 16}}, {{filterLabel("Gravel"), 1}}, INT_MAX},
        {{{filterLabel("Stardust"), 16}}, {{filterLabel("Starmetal Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bone Meal"), 16}}, {{filterLabel("Bone"), 1}}, INT_MAX},
        {{{filterLabel("Clay Dust"), 16}}, {{filterName("minecraft:clay"), 1}}, INT_MAX},
        {{{filterLabel("Bioplastic"), 16}}, {{filterLabel("Sugar Canes"), 2}}, INT_MAX},
        {{{filterLabel("Fluix Dust"), 16}}, {{filterLabel("Fluix Crystal"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Dust"), 16}}, {{filterLabel("Osmium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Soul Powder"), 16}}, {{filterLabel("Soularium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Lithium Dust"), 16}}, {{filterLabel("Lithium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Dust"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Silicon Ingot"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Magnesium Dust"), 16}}, {{filterLabel("Magnesium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Draconium Dust"), 16}}, {{filterLabel("Draconium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Tin"), 16}}, {{filterLabel("Tin Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Enderium Blend"), 16}}, {{filterLabel("Enderium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Coal"), 16}}, {{filterLabel("Coal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Iron"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Lead"), 16}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Gold"), 16}}, {{filterLabel("Gold Ingot"), 1}}, INT_MAX},
        {{{filterLabel("HOP Graphite Dust"), 16}}, {{filterLabel("Coke Dust"), 8}}, INT_MAX},
        {{{filterLabel("Pulverized Silver"), 16}}, {{filterLabel("Silver Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Dust"), 16}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
        {{{filterLabel("Grains of the End"), 16}}, {{filterLabel("Ender Crystal"), 1}}, INT_MAX},
        {{{filterLabel("Nether Quartz Dust"), 16}}, {{filterLabel("Nether Quartz"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Charcoal"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Obsidian"), 16}}, {{filterLabel("Obsidian"), 1}}, INT_MAX},
        {{{filterLabelName("Flour", "appliedenergistics2:material"), 16}}, {{filterLabel("Wheat"), 1}}, INT_MAX}
      }));

    // diamondSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondSieve", "center", "7a5", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Gold Ore"), 16},
          {filterLabel("Boron Ore"), 16},
          {filterLabel("Ardite Ore"), 16},
          {filterLabel("Cobalt Ore"), 16},
          {filterLabel("Lithium Ore"), 16},
          {filterLabel("Thorium Ore"), 16},
          {filterLabel("Magnesium Ore"), 16}
        }, {{filterLabel("Compressed Nether Gravel"), 1, {0}}}, 16},
        {{
          {filterLabel("Ruby"), 16},
          {filterLabel("Topaz"), 16},
          {filterLabel("Amber"), 16},
          {filterLabel("Peridot"), 16},
          {filterLabel("Sapphire"), 16},
          {filterLabel("Tanzanite"), 16},
          {filterLabel("Malachite"), 16}
        }, {{filterLabel("Compressed Ender Gravel"), 1, {0}}}, 16}
      }));

    // ironSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ironSieve", "south", "737", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Uranium Ore"), 16}
        }, {{filterLabel("Compressed Ender Gravel"), 1, {0}}}, 16}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "furnace", "center", "7a5", Actions::north, Actions::down,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:netherbrick"), 16}}, {{filterLabel("Netherrack"), 1}}, INT_MAX},
        {{{filterLabel("Brick"), 16}}, {{filterName("minecraft:clay_ball"), 1}}, INT_MAX},
        {{{filterLabel("Glass"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Stone"), 16}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Bread"), 16}}, {{filterLabelName("Flour", "appliedenergistics2:material"), 1}}, INT_MAX},
        {{{filterLabel("Plastic"), 16}}, {{filterLabel("Dry Rubber"), 1}}, INT_MAX},
        {{{filterLabel("Charcoal"), 16}}, {{filterLabel("Birch Wood"), 1}}, INT_MAX},
        {{{filterLabel("Terracotta"), 16}}, {{filterName("minecraft:clay"), 1}}, INT_MAX},
        {{{filterLabel("Baked Potato"), 16}}, {{filterLabel("Potato"), 1}}, INT_MAX},
        {{{filterLabel("Cactus Green"), 16}}, {{filterLabel("Cactus"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Ingot"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Conduit Binder"), 64}}, {{filterLabel("Conduit Binder Composite"), 1}}, INT_MAX},
        {{{filterLabel("Beryllium Ingot"), 16}}, {{filterLabel("Beryllium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Zirconium Ingot"), 16}}, {{filterLabel("Zirconium Dust"), 1}}, INT_MAX},
        {{{filterLabel("Cooked Tofurkey"), 16}}, {{filterLabel("Raw Tofurkey"), 1}}, INT_MAX},
        {{{filterLabel("Sky Stone Block"), 16}}, {{filterLabel("Sky Stone"), 1}}, INT_MAX},
        {{{filterLabel("HOP Graphite Ingot"), 16}}, {{filterLabel("HOP Graphite Dust"), 1}}, INT_MAX},
        {{{filterLabel("Manganese Oxide Dust"), 16}}, {{filterLabel("Crushed Rhodochrosite"), 1}}, INT_MAX},
        {{{filterLabel("Printed Circuit Board (PCB)"), 8}}, {{filterLabel("Raw Circuit Board"), 1}}, INT_MAX}
      }));

    // terrasteel
    factory.addProcess(std::make_unique<ProcessRedstoneConditional>(factory, "terrasteel", "south", "303", Actions::up, true,
      [&]() { return factory.getAvail(factory.getItem(ItemFilters::Label("Terrasteel Ingot")), true) < 16; }, [](int x) { return !x; },
      std::make_unique<ProcessSlotted>(factory, "terrasteel", "south", "fff", Actions::east, Actions::down,
        std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
          {{{filterLabel("Terrasteel Ingot"), 16}}, {
            {filterLabel("Manasteel Ingot"), 1, {0}},
            {filterLabel("Mana Diamond"), 1, {1}},
            {filterLabel("Mana Pearl"), 1, {2}}
          }, 1}
        })));

    // manaCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "manaCarpenter", "center", "a96", Actions::west, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Wyvern Core"), 16}}, {
          {filterLabel("Ludicrite Ingot"), 2},
          {filterLabel("Draconic Core"), 3},
          {filterLabel("Shulker Shell"), 1},
          {filterLabel("Nether Star"), 1},
          {filterLabel("Pladium"), 1}
        }, INT_MAX},
        {{{filterLabel("Draconic Energy Core"), 16}}, {
          {filterLabel("Awakened Draconium Ingot"), 2},
          {filterLabel("Wyvern Energy Core"), 4},
          {filterLabel("Wyvern Core"), 1}
        }, INT_MAX}
      }));

    // seedOilCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "seedOilCarpenter", "center", "7b7", Actions::down, Actions::up,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Impregnated Stick"), 16}}, {
          {filterLabel("Birch Wood Planks"), 2}
        }, INT_MAX},
        {{{filterLabel("Impregnated Casing"), 16}}, {
          {filterLabel("Birch Wood"), 4}
        }, INT_MAX}
      }));

    // waterCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "waterCarpenter", "center", "a96", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Hardened Casing"), 8}}, {
          {filterLabel("Sturdy Casing"), 1},
          {filterLabel("Diamond"), 4}
        }, INT_MAX},
        {{{filterLabel("Basic Circuit Board"), 8}}, {
          {filterLabel("Tin Ingot"), 1},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Enhanced Circuit Board"), 8}}, {
          {filterLabel("Bronze Ingot"), 1},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Refined Circuit Board"), 8}}, {
          {filterLabel("Iron Ingot"), 1},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Intricate Circuit Board"), 8}}, {
          {filterLabel("Basic Circuit Board"), 1},
          {filterLabel("Enhanced Circuit Board"), 1},
          {filterLabel("Refined Circuit Board"), 1},
          {filterLabel("Printed Engineering Circuit"), 2},
          {filterLabel("Ultimate Control Circuit"), 1}
        }, INT_MAX}
      }));

    // coolantCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "coolantCarpenter", "center", "7f4", Actions::up, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Overclocker Upgrade"), 8}}, {
          {filterLabel("Basic Control Circuit"), 1},
          {filterLabel("Copper Cable"), 1},
          {filterLabel("Tin Plate"), 3}
        }, INT_MAX}
      }));

    // coalCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "coalCarpenter", "center", "80c", Actions::north, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Pedestal"), 16}}, {
          {filterLabel("Black Iron Slate"), 1},
          {filterLabelName("Block of Black Iron", "extendedcrafting:storage"), 1},
          {filterLabel("Black Iron Ingot"), 2}
        }, INT_MAX},
        {{{filterLabel("Crafting Core"), 1}}, {
          {filterLabel("Pedestal"), 2},
          {filterLabel("Osmiridium Ingot"), 2},
          {filterLabel("Crystaltine Component"), 3}
        }, INT_MAX}
      }));

    // dnaCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "dnaCarpenter", "center", "7f4", Actions::east, Actions::down,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Ludicrite Block"), 16}}, {
          {filterLabel("Ender Amethyst"), 1},
          {filterLabel("Blutonium Block"), 1},
          {filterLabel("Alumite Ingot"), 1},
          {filterLabel("Blaze Mesh"), 1},
          {filterLabel("Block of Elementium"), 1},
          {filterLabel("Block of Enderium"), 2}
        }, INT_MAX}
      }));

    // sewageCarpenter
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sewageCarpenter", "west", "303", Actions::south, Actions::north,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Tier 6 Crafting Seed"), 16}}, {
          {filterLabel("Tier 5 Crafting Seed"), 1},
          {filterLabel(u8"§5Insanium Essence"), 2},
          {filterLabel("Seeds"), 4}
        }, INT_MAX}
      }));

    // advancedThermionicFabricator
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "advancedThermionicFabricator", "south", "737", Actions::down, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabelName("Machine Frame", "thermalexpansion:frame"), 1}}, {
          {filterLabel("Enori Crystal"), 4},
          {filterName("rftools:machine_frame"), 1},
          {filterLabel("Heavy Engineering Block"), 1},
          {filterLabel("Iron Casing"), 1},
          {filterLabel("Machine Case"), 1}
        }, INT_MAX},
        {{{filterLabel("Tin Electron Tube"), 16}}, {
          {filterLabel("Tin Ingot"), 5},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Iron Electron Tube"), 16}}, {
          {filterLabel("Iron Ingot"), 5},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Golden Electron Tube"), 16}}, {
          {filterLabel("Gold Ingot"), 5},
          {filterLabel("Redstone"), 2}
        }, INT_MAX},
        {{{filterLabel("Ender Electron Tube"), 16}}, {
          {filterLabel("End Stone"), 5},
          {filterLabel("Eye of Ender"), 2}
        }, INT_MAX},
        {{{filterLabel("Resonant Cell Frame (Empty)"), 1}}, {
          {filterLabel("Mana Dust"), 3},
          {filterLabel("Lumium Ingot"), 2},
          {filterLabel("Hardened Enderium Glass"), 2},
          {filterLabel("Signalum Cell Frame (Full)"), 1},
          {filterLabel("Ender Casing"), 1}
        }, INT_MAX}
      }));

    // throwInLiquidStarlight
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "throwInLiquidStarlight", "center", "7b7", Actions::south, Actions::up,
      std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Infused Wood"), 16}}, {
          {filterLabel("Birch Wood"), 1, {0}}
        }, 16},
        {{{filterLabel("Fluix Crystal"), 16}}, {
          {filterLabel("Charged Certus Quartz Crystal"), 1, {0}},
          {filterLabel("Nether Quartz"), 1, {1}},
          {filterLabel("Redstone"), 1, {2}}
        }, 16}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "center", "777", Actions::west, Actions::north,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Steel Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {0}},
          {filterLabel("Graphite Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Iron Alloy Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {0}},
          {filterLabel("Lead Ingot"), 2, {1}}
        }, 64},
        {{{filterLabel("Bronze Ingot"), 16}}, {
          {filterLabel("Copper Ingot"), 3, {0}},
          {filterLabel("Tin Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Invar Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 2, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Electrum Ingot"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Silver Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Soularium Ingot"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Soul Sand"), 1, {1}}
        }, 64},
        {{{filterLabel("Constantan Ingot"), 16}}, {
          {filterLabel("Copper Ingot"), 1, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Redstone Alloy Ingot"), 16}}, {
          {filterLabel("Redstone"), 1, {0}},
          {filterLabel("Silicon Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Fused Quartz"), 16}}, {
          {filterLabel("Nether Quartz"), 4, {0}},
          {filterLabel("Block of Quartz"), 1, {1}}
        }, 64},
        {{{filterLabel("Energetic Alloy Ingot"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Energetic Blend"), 2, {1}}
        }, 64},
        {{{filterLabel("Vibrant Alloy Ingot"), 16}}, {
          {filterLabel("Energetic Alloy Ingot"), 1, {0}},
          {filterLabel("Ender Pearl"), 1, {1}}
        }, 64},
        {{{filterLabel("Electrical Steel Ingot"), 16}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Silicon Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Shibuichi Alloy"), 16}}, {
          {filterLabel("Copper Ingot"), 3, {0}},
          {filterLabel("Silver Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Lead Platinum Alloy"), 16}}, {
          {filterLabel("Lead Ingot"), 3, {0}},
          {filterLabel("Platinum Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Tin Silver Alloy"), 16}}, {
          {filterLabel("Tin Ingot"), 3, {0}},
          {filterLabel("Silver Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Pulsating Iron Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {0}},
          {filterLabel("Ender Pearl"), 1, {1}}
        }, 64},
        {{{filterLabel("Ferroboron Alloy"), 16}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Boron Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Tough Alloy"), 16}}, {
          {filterLabel("Ferroboron Alloy"), 1, {0}},
          {filterLabel("Lithium Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Hard Carbon Alloy"), 16}}, {
          {filterLabel("Graphite Ingot"), 2, {0}},
          {filterLabel("Diamond"), 1, {1}}
        }, 64},
        {{{filterLabel("Magnesium Diboride Alloy"), 16}}, {
          {filterLabel("Magnesium Ingot"), 1, {0}},
          {filterLabel("Boron Ingot"), 2, {1}}
        }, 64},
        {{{filterLabel("Manyullyn Ingot"), 16}}, {
          {filterLabel("Ardite Ingot"), 1, {0}},
          {filterLabel("Cobalt Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Titanium Aluminide Ingot"), 16}}, {
          {filterLabel("Aluminum Ingot"), 7, {0}},
          {filterLabel("Titanium Ingot"), 3, {1}}
        }, 64},
        {{{filterLabel("Titanium Iridium Alloy Ingot"), 16}}, {
          {filterLabel("Titanium Ingot"), 1, {0}},
          {filterLabel("Iridium Ingot"), 1, {1}}
        }, 64},
        {{{filterLabel("Dark Steel Ingot"), 16}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Obsidian"), 1, {1}}
        }, 64},
        {{{filterLabel("Lithium Manganese Dioxide Alloy"), 16}}, {
          {filterLabel("Lithium Ingot"), 1, {0}},
          {filterLabel("Manganese Dioxide Dust"), 1, {1}}
        }, 64}
      }));

    // alloySmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloySmelter", "south", "a1b", Actions::north, Actions::west,
      std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Organic Brown Dye"), 16}}, {
          {filterLabel("Cocoa Beans"), 2, {0}},
          {filterName("minecraft:slime_ball"), 1, {1}},
          {filterLabel("Pulverized Charcoal"), 2, {2}}
        }, INT_MAX},
        {{{filterLabel("Organic Black Dye"), 16}}, {
          {filterLabel("Pulverized Coal"), 6, {0}},
          {filterName("minecraft:slime_ball"), 1, {1}}
        }, INT_MAX},
        {{{filterLabel("Organic Green Dye"), 16}}, {
          {filterLabel("Cactus Green"), 2, {0}},
          {filterName("minecraft:slime_ball"), 1, {1}},
          {filterLabel("Pulverized Charcoal"), 2, {2}}
        }, INT_MAX},
        {{{filterLabel("End Steel Ingot"), 16}}, {
          {filterLabel("Dark Steel Ingot"), 1, {0}},
          {filterLabel("End Stone"), 1, {1}},
          {filterLabel("Obsidian"), 1, {2}}
        }, INT_MAX},
        {{{filterLabel("Industrial Machine Chassis"), 1}}, {
          {filterLabel("Simple Machine Chassis"), 1, {0}},
          {filterLabel("Industrial Dye Blend"), 1, {1}}
        }, INT_MAX}
      }));

    // isotopeSeparator
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "isotopeSeparator", "south", "a1b", Actions::down, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Uranium-238"), 16},
          {filterLabel("Tiny Clump of Uranium-235"), 16}
        }, {{filterLabel("Uranium Ingot"), 1, {0}}}, 64}
      }));

    // osmiumCompressor
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "osmiumCompressor", "south", "a1b", Actions::up, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Glowstone Ingot"), 16}}, {{filterLabel("Glowstone Dust"), 1, {0}}}, 16},
        {{{filterLabel("Refined Obsidian Ingot"), 16}}, {{filterLabel("Refined Obsidian Dust"), 1, {0}}}, 16}
      }));

    // resonator
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "resonator", "south", "e22", Actions::down, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Stoneburnt"), 16}}, {{filterLabel("Polished Stone"), 1, {0}}}, INT_MAX},
        {{{filterLabel("Lunar Reactive Dust"), 16}}, {{filterLabel("Lapis Lazuli"), 1, {0}}}, INT_MAX}
      }));

    // pinkSlimeIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pinkSlimeIngot", "west", "303", Actions::down, Actions::north,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Pink Slime Ingot"), 16}}, {{filterLabel("Iron Ingot"), 1, {6}}}, 16}
      }));

    // groundTrap
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "groundTrap", "west", "303", Actions::west, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Raw Rabbit"), 16}}, {{filterLabel("Fruit Bait"), 1}}, INT_MAX},
        {{
          {filterLabel("Raw Turkey"), 16},
          {filterLabel("Raw Beef"), 16}
        }, {{filterLabel("Grain Bait"), 1}}, INT_MAX},
        {{
          {filterLabel("Raw Venison"), 16},
          {filterLabel("Raw Chicken"), 16},
          {filterLabel("Raw Mutton"), 16},
          {filterLabel("Raw Duck"), 16}
        }, {{filterLabel("Veggie Bait"), 1}}, INT_MAX}
      }));

    // calciumSulfate
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "calciumSulfate", "south", "e22", Actions::west, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Calcium Sulfate"), 16}}, {{filterLabel("Crushed Fluorite"), 1, {0}}}, 16}
      }));

    // boronNitride
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "boronNitride", "east", "1d7", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Hexagonal Boron Nitride"), 16}}, {{filterLabel("Boron Ingot"), 1, {0}}}, 16}
      }));

    // draconiumCrystal
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "draconiumCrystal", "east", "1d7", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Draconium Crystal"), 16}}, {{filterLabel("Draconium Ore"), 1, {0}}}, 16}
      }));

    // compressedHammer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressedHammer", "center", "312", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Crushed Netherrack"), 16}}, {{filterLabel("Compressed Netherrack"), 1, {0}}}, 16},
        {{{filterLabel("Crushed Endstone"), 16}}, {{filterLabel("Compressed End Stone"), 1, {0}}}, 16},
        {{{filterLabel("Gravel"), 16}}, {{filterLabel("Compressed Cobblestone"), 1, {0}}}, 16},
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

    // extruder
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "extruder", "center", "539", Actions::north, Actions::up,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Shaft (Iron)"), 16}}, {{filterLabel("Block of Iron"), 1, {6}}}, 16},
        {{{filterLabel("Copper Cable"), 16}}, {{filterLabel("Copper Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Gold Cable"), 16}}, {{filterLabel("Gold Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Tin Cable"), 16}}, {{filterLabel("Tin Ingot"), 1, {6}}}, 16}
      }));

    // roller
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "roller", "center", "539", Actions::west, Actions::up,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Bronze Item Casing"), 16}}, {{filterLabel("Bronze Plate"), 1, {6}}}, 16},
        {{{filterLabel("Copper Item Casing"), 16}}, {{filterLabel("Copper Plate"), 1, {6}}}, 16},
        {{{filterLabel("Iron Item Casing"), 16}}, {{filterLabel("Iron Plate"), 1, {6}}}, 16},
        {{{filterLabel("Lead Item Casing"), 16}}, {{filterLabel("Lead Plate"), 1, {6}}}, 16},
        {{{filterLabel("Tin Item Casing"), 16}}, {{filterLabel("Tin Plate"), 1, {6}}}, 16}
      }));

    // treatedWood
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "treatedWood", "center", "312", Actions::west, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Treated Wood Planks"), 64}}, {{filterLabel("Birch Wood Planks"), 1, {0}}}, 64}
      }));

    // terraCrystal
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "terraCrystal", "center", "312", Actions::north, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Terra Vis Crystal"), 16}}, {{filterLabel("Quartz Sliver"), 1, {0}}}, 16}
      }));

    // brassIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "brassIngot", "east", "5a8", Actions::east, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Alchemical Brass Ingot"), 16}}, {{filterLabel("Iron Ingot"), 1, {0}}}, 16}
      }));

    // sandInduction
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sandInduction", "south", "a1b", Actions::east, Actions::west,
      std::vector<StockEntry>{}, 64, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Rich Slag"), 64}}, {{filterLabel("Clock"), 1}}, INT_MAX},
        {{{filterLabel("Tin Ingot"), 64}}, {{filterLabel("Tin Ore"), 1}}, INT_MAX},
        {{{filterLabel("Gold Ingot"), 64}}, {{filterLabel("Gold Ore"), 1}}, INT_MAX},
        {{{filterLabel("Lead Ingot"), 64}}, {{filterLabel("Lead Ore"), 1}}, INT_MAX},
        {{{filterLabel("Iron Ingot"), 64}}, {{filterLabel("Iron Ore"), 1}}, INT_MAX},
        {{{filterLabel("Boron Ingot"), 64}}, {{filterLabel("Boron Ore"), 1}}, INT_MAX},
        {{{filterLabel("Copper Ingot"), 64}}, {{filterLabel("Copper Ore"), 1}}, INT_MAX},
        {{{filterLabel("Silver Ingot"), 64}}, {{filterLabel("Silver Ore"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Ingot"), 64}}, {{filterLabel("Osmium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Ore"), 1}}, INT_MAX},
        {{{filterLabel("Ardite Ingot"), 64}}, {{filterLabel("Ardite Ore"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Ingot"), 64}}, {{filterLabel("Uranium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Lithium Ingot"), 64}}, {{filterLabel("Lithium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Thorium Ingot"), 64}}, {{filterLabel("Thorium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Ingot"), 64}}, {{filterLabel("Aluminum Ore"), 1}}, INT_MAX},
        {{{filterLabel("Magnesium Ingot"), 64}}, {{filterLabel("Magnesium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Draconium Ingot"), 64}}, {{filterLabel("Draconium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Starmetal Ingot"), 64}}, {{filterLabel("Starmetal Ore"), 1}}, INT_MAX},
        {{{filterLabel("Mana Infused Ingot"), 64}}, {{filterLabel("Mana Infused Ore"), 1}}, INT_MAX}
      }));

    // inductionSmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "inductionSmelter", "center", "539", Actions::south, Actions::up,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Nickel Ingot"), 16},
          {filterLabel("Platinum Ingot"), 16}
        }, {
          {filterLabel("Nickel Ore"), 1, {0}},
          {filterLabel("Cinnabar"), 1, {1}}
        }, 16},
        {{
          {filterLabel("Platinum Ingot"), 16},
          {filterLabel("Iridium Ingot"), 16}
        }, {
          {filterLabel("Platinum Ore"), 1, {0}},
          {filterLabel("Cinnabar"), 1, {1}}
        }, 16},
        {{{filterLabel("Iridium Ingot"), 16}}, {
          {filterLabel("Iridium Ore"), 1, {0}},
          {filterLabel("Cinnabar"), 1, {1}}
        }, 16},
        {{{filterLabel("Black Iron Ingot"), 16}}, {
          {filterLabel("Block of Invar"), 1, {0}},
          {filterLabel("Tough Alloy"), 1, {1}}
        }, 16},
        {{{filterLabel("Signalum Cell Frame (Full)"), 4}}, {
          {filterLabel("Signalum Cell Frame (Empty)"), 1, {0}},
          {filterLabel("Block of Redstone"), 40, {1}}
        }, 40},
        {{{filterLabel("Hardened Glass"), 16}}, {
          {filterLabel("Pulverized Lead"), 1, {0}},
          {filterLabel("Pulverized Obsidian"), 4, {1}}
        }, 64},
        {{{filterLabel("Soul Machine Chassis"), 1}}, {
          {filterLabel("Simple Machine Chassis"), 1, {0}},
          {filterLabel("Soul Attuned Dye Blend"), 1, {1}}
        }, 16},
        {{{filterLabel("Aluminum Brass Ingot"), 16}}, {
          {filterLabel("Aluminum Ingot"), 3, {0}},
          {filterLabel("Copper Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Hardened Enderium Glass"), 16}}, {
          {filterLabel("Enderium Blend"), 1, {0}},
          {filterLabel("Hardened Glass"), 2, {1}}
        }, 16}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "charger", "center", "ffd", Actions::north, Actions::south,
      std::vector<StockEntry>{}, 256, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1}}, INT_MAX},
        {{{filterLabel("Charged Certus Quartz Crystal"), 16}}, {{filterLabel("Certus Quartz Crystal"), 1}}, INT_MAX}
      }));

    // ironberries
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ironberries", "west", "9d8", Actions::west, Actions::east,
      std::vector<size_t>{10}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Ironberries"), 64},
          {filterLabel("Ironwood"), 64}
        }, {{filterLabel("Ironwood Sapling"), 1, {10}}}, 16}
      }));

    // pulverizer
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pulverizer", "center", "ffd", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("harvestcraft:flouritem"), 16}}, {{filterLabel("Wheat"), 1}}, INT_MAX},
        {{{filterLabel("Dandelion Yellow"), 16}}, {{filterLabel("Dandelion"), 1}}, INT_MAX},
        {{{filterLabel("Ground Cinnamon"), 16}}, {{filterLabel("Cinnamon"), 1}}, INT_MAX},
        {{}, {{filterLabel("Aquamarine Shale"), 1}}, INT_MAX},
        {{}, {{filterLabel("Redstone Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Apatite Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Coal Ore"), 1}}, INT_MAX}
      }));

    // centrifuge
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "centrifuge", "center", "1dc", Actions::west, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Pulsating Propolis"), 16}}, {{filterLabel("Mysterious Comb"), 1, {0}}}, 64},
        {{{filterLabel("Fruit Bait"), 16}}, {{filterLabel("Strawberry"), 1, {0}}}, 64},
        {{{filterLabel("Sugar"), 16}}, {{filterLabel("Sugar Canes"), 1, {0}}}, 64},
        {{
          {filterLabel("Silken Tofu"), 16},
          {filterLabel("Grain Bait"), 16}
        }, {{filterLabel("Soybean"), 1, {0}}}, 64},
        {{
          {filterLabel("Firm Tofu"), 16},
          {filterLabel("Soy Milk"), 16}
        }, {{filterLabel("Silken Tofu"), 1, {0}}}, 64},
        {{
          {filterLabel("Cooking Oil"), 16},
          {filterLabel("Veggie Bait"), 16}
        }, {{filterLabel("Pumpkin"), 1, {0}}}, 64}
      }));

    // enchanter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enchanter", "center", "7a5", Actions::up, Actions::down,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Evil Infused Iron Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 8, {0}},
          {filterLabel("Nether Star"), 1, {1}}
        }, 64},
        {{{filterLabel("Enchanted Ingot"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Lapis Lazuli"), 1, {1}}
        }, 64},
        {{{filterLabel("Magical Wood"), 16}}, {
          {filterLabel("Oak Bookshelf"), 1, {0}},
          {filterLabel("Lapis Lazuli"), 1, {1}}
        }, 64}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "539", Actions::down, Actions::up,
      std::vector<StockEntry>{}, 64, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§eInferium Essence"), 256}}, {{filterName("mysticalagriculture:tier4_inferium_seeds"), 1}}, INT_MAX},
        {{{filterName("minecraft:brown_mushroom"), 64}}, {{filterName("minecraft:brown_mushroom"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Wheat"), 64}, {filterLabel("Seeds"), 64}}, {{filterLabel("Seeds"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Essence"), 64}}, {{filterLabel("Lapis Lazuli Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Fiery Ingot Essence"), 64}}, {{filterLabel("Fiery Ingot Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Knightmetal Essence"), 64}}, {{filterLabel("Knightmetal Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Knightslime Essence"), 64}}, {{filterLabel("Knightslime Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Void Metal Essence"), 64}}, {{filterLabel("Void Metal Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Experience Essence"), 64}}, {{filterLabel("Experience Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Brussel Sprout"), 64}}, {{filterLabel("Brussel Sprout Seed"), 1}}, INT_MAX},
        {{{filterLabel("Water Chestnut"), 64}}, {{filterLabel("Water Chestnut Seed"), 1}}, INT_MAX},
        {{{filterLabel("Sky Stone Essence"), 64}}, {{filterLabel("Sky Stone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nether Wart"), 64}}, {{filterLabel("Nether Wart"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Sugar Canes"), 64}}, {{filterLabel("Sugar Canes"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Cocoa Beans"), 64}}, {{filterLabel("Cocoa Beans"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Winter Squash"), 64}}, {{filterLabel("Winter Squash Seed"), 1}}, INT_MAX},
        {{{filterName("minecraft:melon_block"), 64}}, {{filterLabel("Melon Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Steeleaf Essence"), 64}}, {{filterLabel("Steeleaf Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Ironwood Essence"), 64}}, {{filterLabel("Ironwood Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 64}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Thaumium Essence"), 64}}, {{filterLabel("Thaumium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Skeleton Essence"), 64}}, {{filterLabel("Skeleton Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Bamboo Shoot"), 64}}, {{filterLabel("Bamboo Shoot Seed"), 1}}, INT_MAX},
        {{{filterLabel("Sweet Potato"), 64}}, {{filterLabel("Sweet Potato Seed"), 1}}, INT_MAX},
        {{{filterLabel("Gooseberry"), 64}}, {{filterLabel("Gooseberry Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Dandelion"), 64}}, {{filterLabel("Dandelion"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Creeper Essence"), 64}}, {{filterLabel("Creeper Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Essence"), 64}}, {{filterLabel("Uranium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Cauliflower"), 64}}, {{filterLabel("Cauliflower Seed"), 1}}, INT_MAX},
        {{{filterLabel("Copper Essence"), 64}}, {{filterLabel("Copper Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Sulfur Essence"), 64}}, {{filterLabel("Sulfur Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Basalt Essence"), 64}}, {{filterLabel("Basalt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Zombie Essence"), 64}}, {{filterLabel("Zombie Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Chorus Fruit"), 64}}, {{filterLabel("Chorus Flower"), 1}}, INT_MAX},
        {{{filterLabel("Bellpepper"), 64}}, {{filterLabel("Bellpepper Seed"), 1}}, INT_MAX},
        {{{filterLabel("Spice Leaf"), 64}}, {{filterLabel("Spice Leaf Seed"), 1}}, INT_MAX},
        {{{filterLabel("Strawberry"), 64}}, {{filterLabel("Strawberry Seed"), 1}}, INT_MAX},
        {{{filterLabel("Blackberry"), 64}}, {{filterLabel("Blackberry Seed"), 1}}, INT_MAX},
        {{{filterLabel("Cranberry"), 64}}, {{filterLabel("Cranberry Seed"), 1}}, INT_MAX},
        {{{filterLabel("Blueberry"), 64}}, {{filterLabel("Blueberry Seed"), 1}}, INT_MAX},
        {{{filterLabel("Raspberry"), 64}}, {{filterLabel("Raspberry Seed"), 1}}, INT_MAX},
        {{{filterLabel("Artichoke"), 64}}, {{filterLabel("Artichoke Seed"), 1}}, INT_MAX},
        {{{filterLabel("Asparagus"), 64}}, {{filterLabel("Asparagus Seed"), 1}}, INT_MAX},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Birch Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Quicksilver"), 64}}, {{filterLabel("Shimmerleaf"), 1}}, INT_MAX},
        {{{filterLabel("Ender Pearl"), 64}}, {{filterLabel("Ender Lilly"), 1}}, INT_MAX},
        {{{filterLabel("Beetroot"), 64}}, {{filterLabel("Beetroot Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Cactus"), 64}}, {{filterLabel("Cactus"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Potato"), 64}}, {{filterLabel("Potato"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Carrot"), 64}}, {{filterLabel("Carrot"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Iron Essence"), 64}}, {{filterLabel("Iron Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Coal Essence"), 64}}, {{filterLabel("Coal Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Broccoli"), 64}}, {{filterLabel("Broccoli Seed"), 1}}, INT_MAX},
        {{{filterLabel("Zucchini"), 64}}, {{filterLabel("Zucchini Seed"), 1}}, INT_MAX},
        {{{filterLabel("Cucumber"), 64}}, {{filterLabel("Cucumber Seed"), 1}}, INT_MAX},
        {{{filterLabel("Rutabaga"), 64}}, {{filterLabel("Rutabaga Seed"), 1}}, INT_MAX},
        {{{filterLabel("Scallion"), 64}}, {{filterLabel("Scallion Seed"), 1}}, INT_MAX},
        {{{filterLabel("Eggplant"), 64}}, {{filterLabel("Eggplant Seed"), 1}}, INT_MAX},
        {{{filterLabel("Tin Essence"), 64}}, {{filterLabel("Tin Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Dye Essence"), 64}}, {{filterLabel("Dye Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Pumpkin"), 64}}, {{filterLabel("Pumpkin Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Seaweed"), 64}}, {{filterLabel("Seaweed Seed"), 1}}, INT_MAX},
        {{{filterLabel("Spinach"), 64}}, {{filterLabel("Spinach Seed"), 1}}, INT_MAX},
        {{{filterLabel("Cabbage"), 64}}, {{filterLabel("Cabbage Seed"), 1}}, INT_MAX},
        {{{filterLabel("Rhubarb"), 64}}, {{filterLabel("Rhubarb Seed"), 1}}, INT_MAX},
        {{{filterLabel("Parsnip"), 64}}, {{filterLabel("Parsnip Seed"), 1}}, INT_MAX},
        {{{filterLabel("Soybean"), 64}}, {{filterLabel("Soybean Seed"), 1}}, INT_MAX},
        {{{filterLabel("Lettuce"), 64}}, {{filterLabel("Lettuce Seed"), 1}}, INT_MAX},
        {{{filterLabel("Apple"), 64}}, {{filterLabel("Apple Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Radish"), 64}}, {{filterLabel("Radish Seed"), 1}}, INT_MAX},
        {{{filterLabel("Turnip"), 64}}, {{filterLabel("Turnip Seed"), 1}}, INT_MAX},
        {{{filterLabel("Celery"), 64}}, {{filterLabel("Celery Seed"), 1}}, INT_MAX},
        {{{filterLabel("Tomato"), 64}}, {{filterLabel("Tomato Seed"), 1}}, INT_MAX},
        {{{filterLabel("Garlic"), 64}}, {{filterLabel("Garlic Seed"), 1}}, INT_MAX},
        {{{filterLabel("Grapes"), 64}}, {{filterLabel("Grape Seed"), 1}}, INT_MAX},
        {{{filterLabel("Onion"), 64}}, {{filterLabel("Onion Seed"), 1}}, INT_MAX},
        {{{filterLabel("Flax"), 64}}, {{filterLabel("Flax Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Bean"), 64}}, {{filterLabel("Bean Seed"), 1}}, INT_MAX},
        {{{filterLabel("Corn"), 64}}, {{filterLabel("Corn Seed"), 1}}, INT_MAX},
        {{{filterLabel("Leek"), 64}}, {{filterLabel("Leek Seed"), 1}}, INT_MAX},
        {{{filterLabel("Okra"), 64}}, {{filterLabel("Okra Seed"), 1}}, INT_MAX},
        {{{filterLabel("Peas"), 64}}, {{filterLabel("Peas Seed"), 1}}, INT_MAX}
      }));

    // cinnamon
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cinnamon", "center", "80c", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Cinnamon"), 16}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // wildberries
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "wildberries", "west", "9d8", Actions::south, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Wildberries"), 16}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 16}
      }));

    // enrichment
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "enrichment", "center", "777", Actions::up, Actions::north,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Diamond Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Emerald Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Prosperity Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Black Quartz Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Lapis Lazuli Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Nether Quartz Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Certus Quartz Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Dimensional Shard Ore"), 1}}, INT_MAX},
        {{}, {{filterLabel("Charged Certus Quartz Ore"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Dust"), 64}}, {{filterLabel("Glowstone"), 1}}, INT_MAX},
        {{{filterLabel("Compressed Diamond"), 1}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Compressed Redstone"), 1}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Pure Fluix Crystal"), 16}}, {{filterLabel("Fluix Seed"), 1}}, INT_MAX},
        {{{filterLabel("Compressed Obsidian"), 1}}, {{filterLabel("Refined Obsidian Dust"), 1}}, INT_MAX},
        {{{filterLabel("Pure Certus Quartz Crystal"), 16}}, {{filterLabel("Certus Quartz Seed"), 1}}, INT_MAX},
        {{{filterLabel("Pure Nether Quartz Crystal"), 16}}, {{filterLabel("Nether Quartz Seed"), 1}}, INT_MAX}
      }));

    // redstoneInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneInfuser", "center", "0ed", Actions::south, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enriched Alloy"), 32}}, {{filterLabel("Iron Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Energy Cell Frame"), 1}}, {{filterLabelName("Machine Frame", "thermalexpansion:frame"), 1, {0}}}, 16},
        {{{filterLabel("Basic Control Circuit"), 32}}, {{filterLabel("Osmium Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Reception Coil"), 8}}, {{filterLabel("Gold Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Conductance Coil"), 8}}, {{filterLabel("Electrum Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Transmission Coil"), 8}}, {{filterLabel("Silver Ingot"), 1, {0}}}, 16}
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

    // bioInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "bioInfuser", "east", "5a8", Actions::down, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Dirt"), 64}}, {{filterLabel("Sand"), 1, {0}}}, 16}
      }));

    // fungiInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "fungiInfuser", "east", "5a8", Actions::west, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Mycelium"), 64}}, {{filterLabel("Dirt"), 1, {0}}}, INT_MAX}
      }));

    // pressurizer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pressurizer", "south", "547", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Cubic Boron Nitride"), 16}}, {{filterLabel("Hexagonal Boron Nitride"), 1, {0}}}, 16},
        {{{filterLabel("Dense Copper Plate"), 16}}, {{filterLabel("Copper Plate"), 9, {0}}}, 18},
        {{{filterLabel("Dense Steel Plate"), 16}}, {{filterLabel("Steel Plate"), 9, {0}}}, 18},
        {{{filterLabel("Dense Lead Plate"), 16}}, {{filterLabel("Lead Plate"), 9, {0}}}, 18},
        {{{filterLabel("Dense Iron Plate"), 16}}, {{filterLabel("Iron Plate"), 9, {0}}}, 18},
        {{{filterLabel("Dense Gold Plate"), 16}}, {{filterLabel("Gold Plate"), 9, {0}}}, 18},
        {{{filterLabel("Dense Tin Plate"), 16}}, {{filterLabel("Tin Plate"), 9, {0}}}, 18},
        {{{filterLabel("Silicon Dioxide"), 16}}, {{filterLabel("Clay Dust"), 4, {0}}}, 16},
        {{{filterLabel("Rhodochrosite"), 16}}, {{filterLabel("Crushed Rhodochrosite"), 1, {0}}}, 16},
        {{{filterLabel("Fluorite"), 16}}, {{filterLabel("Crushed Fluorite"), 1, {0}}}, 16}
      }));

    // crafterA
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafterA", "center", "777", Actions::south, Actions::north,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Salt"), 256}}, {{filterLabel("Fresh Water"), 1}}, INT_MAX},
        {{{filterLabel("Salt"), 256}}, {{filterLabel("Fresh Water"), 1}}, INT_MAX},
        {{{filterLabel("Salt"), 256}}, {{filterLabel("Fresh Water"), 1}}, INT_MAX},
        {{{filterLabel("Cranberry Jelly"), 16}}, {
          {filterLabel("Cranberry"), 1},
          {filterLabel("Sugar"), 1}
        }, INT_MAX},
        {{{filterLabel("Butter"), 16}}, {
          {filterLabel("Silken Tofu"), 1},
          {filterLabel("Salt"), 1}
        }, INT_MAX},
        {{{filterLabel("Sandstone"), 16}}, {
          {filterLabel("Sand"), 4}
        }, INT_MAX},
        {{{filterLabel("Rich Phyto-Gro"), 64}}, {
          {filterLabel("Pulverized Charcoal"), 1},
          {filterLabel("Niter"), 1},
          {filterLabel("Rich Slag"), 1}
        }, INT_MAX},
        {{{filterLabel("Buttered Potato"), 16}}, {
          {filterLabel("Baked Potato"), 1},
          {filterLabel("Butter"), 1}
        }, INT_MAX},
        {{{filterLabel("Energy Tablet"), 8}}, {
          {filterLabel("Redstone"), 4},
          {filterLabel("Enriched Alloy"), 2},
          {filterLabel("Gold Ingot"), 3}
        }, INT_MAX},
        {{}, {
          {filterLabel("Nether Star Shard"), 3}
        }, INT_MAX}
      }));

    // crafterB
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafterB", "south", "e22", Actions::up, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§aPrudentium Essence"), 40}}, {{filterLabel(u8"§eInferium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§6Intermedium Essence"), 40}}, {{filterLabel(u8"§aPrudentium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§6Intermedium Essence"), 40}}, {{filterLabel(u8"§aPrudentium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§bSuperium Essence"), 40}}, {{filterLabel(u8"§6Intermedium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§cSupremium Essence"), 40}}, {{filterLabel(u8"§bSuperium Essence"), 4}}, INT_MAX},
        {{{filterLabel(u8"§5Insanium Essence"), 40}}, {{filterLabel(u8"§cSupremium Essence"), 4}}, INT_MAX},
        {{{filterLabel("Marshmallows"), 16}}, {
          {filterLabel("Sugar"), 1},
          {filterLabel("Fresh Water"), 1},
          {filterLabel("Raw Tofeeg"), 1}
        }, INT_MAX},
        {{{filterLabel("Dough"), 16}}, {
          {filterLabel("Fresh Water"), 1},
          {filterName("harvestcraft:flouritem"), 1},
          {filterLabel("Salt"), 1}
        }, INT_MAX},
        {{{filterLabel("Tin Ingot"), 64}}, {
          {filterLabel("Tin Essence"), 8}
        }, INT_MAX}
      }));

    // crafterC
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafterC", "west", "cfa", Actions::east, Actions::west,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Magnesium Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Lithium Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Ardite Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Cobalt Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Cryotheum Dust"), 64}}, {
          {filterLabel("Blizz Powder"), 2},
          {filterLabel("Redstone"), 1},
          {filterLabel("Snowball"), 1}
        }, INT_MAX},
        {{{filterLabel("Uranium Ingot"), 64}}, {
          {filterLabel("Uranium Essence"), 8}
        }, INT_MAX},
        {{{filterLabel("Clock"), 64}}, {
          {filterLabel("Gold Ingot"), 4},
          {filterLabel("Redstone"), 1}
        }, INT_MAX},
        {{{filterLabel("Neutron Reflector"), 16}}, {
          {filterLabel("Pulverized Tin"), 4},
          {filterLabel("Pulverized Coal"), 4},
          {filterLabel("Copper Plate"), 1}
        }, INT_MAX}
      }));

    // crafterD
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafterD", "east", "caf", Actions::east, Actions::north,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Compressed Cobblestone"), 64}}, {{filterLabel("Cobblestone"), 9}}, INT_MAX},
        {{{filterLabel("Block of Quartz"), 64}}, {{filterLabel("Nether Quartz"), 4}}, INT_MAX},
        {{{filterLabel("Block of Coal"), 64}}, {{filterLabel("Coal"), 9}}, INT_MAX},
        {{}, {{filterLabel("Gold Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Boron Ore Piece"), 4}}, INT_MAX},
        {{}, {{filterLabel("Thorium Ore Piece"), 4}}, INT_MAX},
        {{{filterLabel("Diamond Hammer"), 9}}, {
          {filterLabel("Diamond"), 2},
          {filterLabel("Stick"), 2}
        }, INT_MAX},
        {{{filterLabel("Piston"), 64}}, {
          {filterLabel("Treated Wood Planks"), 3},
          {filterLabel("Compressed Cobblestone"), 4},
          {filterLabel("Aluminum Plate"), 1},
          {filterLabel("Redstone"), 1}
        }, INT_MAX}
      }));

    // crafterE
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "crafterE", "west", "10b", Actions::down, Actions::south,
      std::vector<StockEntry>{}, INT_MAX, [](size_t slot) { return slot < 12; }, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Raw Prime Steak"), 16}}, {{filterLabel("Raw Prime Beef"), 1}}, INT_MAX},
        {{{filterLabel("Raw Prime Bacon"), 16}}, {{filterLabel("Raw Prime Pork"), 1}}, INT_MAX}
      }));

    // quantumCompressor
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "quantumCompressor", "west", "625", Actions::down, Actions::up,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Matter Condenser"), 1}}, {{filterLabel("Piston"), 1}}, INT_MAX},
        {{{filterLabel("Tin Singularity"), 1}}, {{filterLabel("Tin Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Emerald Singularity"), 1}}, {{filterLabel("Emerald"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Singularity"), 1}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Iron Singularity"), 1}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Gold Singularity"), 1}}, {{filterLabel("Gold Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Lead Singularity"), 1}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Invar Singularity"), 1}}, {{filterLabel("Invar Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Steel Singularity"), 1}}, {{filterLabel("Steel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Coal Singularity"), 1}}, {{filterLabel("Block of Coal"), 1}}, INT_MAX},
        {{{filterLabel("Lumium Singularity"), 1}}, {{filterLabel("Lumium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Nickel Singularity"), 1}}, {{filterLabel("Nickel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Singularity"), 1}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Silver Singularity"), 1}}, {{filterLabel("Silver Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bronze Singularity"), 1}}, {{filterLabel("Bronze Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Singularity"), 1}}, {{filterLabel("Cobalt Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Ardite Singularity"), 1}}, {{filterLabel("Ardite Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Singularity"), 1}}, {{filterLabel("Glowstone"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Singularity"), 1}}, {{filterLabel("Uranium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Quartz Singularity"), 1}}, {{filterLabel("Block of Quartz"), 1}}, INT_MAX},
        {{{filterLabel("Signalum Singularity"), 1}}, {{filterLabel("Signalum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Singularity"), 1}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Electrum Singularity"), 1}}, {{filterLabel("Electrum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Black Quartz Singularity"), 1}}, {{filterLabel("Black Quartz"), 1}}, INT_MAX},
        {{{filterLabel("Flux Crystal Singularity"), 1}}, {{filterLabel("Flux Crystal"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Singularity"), 1}}, {{filterLabel("Block of Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Mithril Singularity"), 1}}, {{filterLabel("Mana Infused Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Constantan Singularity"), 1}}, {{filterLabel("Constantan Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Singularity"), 1}}, {{filterLabel("Lapis Lazuli Block"), 1}}, INT_MAX},
        {{{filterLabel("Certus Quartz Singularity"), 1}}, {{filterLabel("Certus Quartz Crystal"), 1}}, INT_MAX}
      }));

    // alchemy
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "alchemy", "south", "737", Actions::south, Actions::up,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterName("minecraft:slime_ball"), 16}}, {{filterLabel("Cactus"), 1}}, INT_MAX},
        {{{filterLabel("Manasteel Ingot"), 16}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Dust"), 16}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Mana Diamond"), 16}}, {{filterLabel("Diamond"), 1}}, INT_MAX},
        {{{filterLabel("Mana Pearl"), 16}}, {{filterLabel("Ender Pearl"), 1}}, INT_MAX}
      }));

    // conjuration
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "conjuration", "south", "fff", Actions::south, Actions::down,
      std::vector<StockEntry>{}, 64, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Mana Powder"), 16}}, {{filterLabel("Gunpowder"), 1}}, INT_MAX},
        {{{filterLabel("Psigem"), 64}}, {{filterLabel("Psigem"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Psidust"), 64}}, {{filterLabel("Psidust"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Netherrack"), 256}}, {{filterLabel("Netherrack"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Psimetal Ingot"), 64}}, {{filterLabel("Psimetal Ingot"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Ebony Substance"), 64}}, {{filterLabel("Ebony Substance"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Ivory Substance"), 64}}, {{filterLabel("Ivory Substance"), 1, {}, true}}, INT_MAX}
      }));

    // elven
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "elven", "south", "737", Actions::west, Actions::up,
      std::vector<StockEntry>{}, 32, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Elementium Ingot"), 16}}, {{filterLabel("Manasteel Ingot"), 2}}, INT_MAX},
        {{{filterLabel("Dragonstone"), 16}}, {{filterLabel("Mana Diamond"), 1}}, INT_MAX}
      }));

    // starlightTransmutation
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "starlightTransmutation", "south", "fff", Actions::west, Actions::down,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("End Stone"), 16}}, {{filterLabel("Sandstone"), 1, {}, true}}, INT_MAX},
        {{{filterLabel("Starmetal Ore"), 16}}, {{filterLabel("Iron Ore"), 1, {}, true}}, INT_MAX}
      }));

    // redstoneFluidInfuser
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneFluidInfuser", "south", "cb8", Actions::west, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Signalum Ingot"), 16}}, {{filterLabel("Shibuichi Alloy"), 1, {0}}}, INT_MAX},
        {{{filterLabel("Fluxed Electrum Ingot"), 16}}, {{filterLabel("Electrum Ingot"), 1, {0}}}, INT_MAX}
      }));

    // redstoneTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "redstoneFluidTransposer", "south", "00a", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Flux Crystal"), 16}}, {{filterLabel("Diamond"), 1, {0}}}, INT_MAX},
        {{{filterLabel("Wyvern Energy Core"), 16}}, {{filterLabel("Draconic Core"), 1, {0}}}, INT_MAX}
      }));

    // cryotheumTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cryotheumTransposer", "south", "06e", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{}, {{filterLabel("Cinnabar Ore"), 1, {0}}}, 64}
      }));

    // enderium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enderium", "south", "cb8", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enderium Ingot"), 16}}, {{filterLabel("Lead Platinum Alloy"), 1, {0}}}, 16}
      }));

    // lumium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "lumium", "south", "cb8", Actions::south, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Lumium Ingot"), 16}}, {{filterLabel("Tin Silver Alloy"), 1, {0}}}, 64}
      }));

    // xpTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "xpTransposer", "south", "06e", Actions::south, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Blaze Powder"), 16}}, {{filterLabel("Sulfur"), 2, {0}}}, INT_MAX},
        {{{filterLabel("Blitz Powder"), 16}}, {{filterLabel("Niter"), 2, {0}}}, INT_MAX},
        {{{filterLabel("Blizz Powder"), 16}}, {{filterLabel("Snowball"), 2, {0}}}, INT_MAX},
        {{{filterLabel("Basalz Powder"), 16}}, {{filterLabel("Pulverized Obsidian"), 2, {0}}}, INT_MAX}
      }));

    // reinforcedCellFrame
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "reinforcedCellFrame", "south", "00a", Actions::south, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Reinforced Cell Frame (Full)"), 1}}, {{filterLabel("Reinforced Cell Frame (Empty)"), 1, {0}}}, 16}
      }));

    // siliconInscriber
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "siliconInscriber", "south", "9ac", Actions::west, Actions::down,
      std::vector<size_t>{1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Printed Silicon"), 16}}, {{filterLabel("Silicon Ingot"), 1, {1}}}, 16}
      }));

    // certusInscriber
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "certusInscriber", "south", "9ac", Actions::east, Actions::down,
      std::vector<size_t>{1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Printed Calculation Circuit"), 16}}, {{filterLabel("Pure Certus Quartz Crystal"), 1, {1}}}, 16}
      }));

    // diamondInscriber
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondInscriber", "south", "5a8", Actions::south, Actions::up,
      std::vector<size_t>{1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Printed Engineering Circuit"), 16}}, {{filterLabel("Diamond"), 1, {1}}}, 16}
      }));

    // goldInscriber
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondInscriber", "south", "5a8", Actions::east, Actions::up,
      std::vector<size_t>{1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Printed Logic Circuit"), 16}}, {{filterLabel("Gold Ingot"), 1, {1}}}, 16}
      }));

    // processorInscriber
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "processorInscriber", "south", "9ac", Actions::south, Actions::down,
      std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Engineering Processor"), 16}}, {
          {filterLabel("Printed Engineering Circuit"), 1, {0}},
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Printed Silicon"), 1, {2}}
        }, 16},
        {{{filterLabel("Logic Processor"), 16}}, {
          {filterLabel("Printed Logic Circuit"), 1, {0}},
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Printed Silicon"), 1, {2}}
        }, 16},
        {{{filterLabel("Calculation Processor"), 16}}, {
          {filterLabel("Printed Calculation Circuit"), 1, {0}},
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Printed Silicon"), 1, {2}}
        }, 16}
      }));

    // ultimateIngotA
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotA", "south", "9ac", Actions::up, Actions::down,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotA"), 1}}, {
          {filterLabel("Lithium Manganese Dioxide Alloy"), 1, {0}},
          {filterLabel("Titanium Iridium Alloy Ingot"), 1, {1}},
          {filterLabel("Titanium Aluminide Ingot"), 1, {2}},
          {filterLabel("Awakened Draconium Ingot"), 1, {3}},
          {filterLabel("Magnesium Diboride Alloy"), 1, {4}},
          {filterLabel("Evil Infused Iron Ingot"), 1, {5}},
          {filterLabel("Refined Obsidian Ingot"), 1, {6}},
          {filterLabel("HOP Graphite Ingot"), 1, {7}},
          {filterLabel("Mana Infused Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotB
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotB", "west", "9d8", Actions::up, Actions::east,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotB"), 1}}, {
          {filterLabel("Crystaltine Ingot"), 1, {0}},
          {filterLabel("Hard Carbon Alloy"), 1, {1}},
          {filterLabel("Terrasteel Ingot"), 1, {2}},
          {filterLabel("Elementium Ingot"), 1, {3}},
          {filterLabel("Black Iron Ingot"), 1, {4}},
          {filterLabel("Ferroboron Alloy"), 1, {5}},
          {filterLabel("Starmetal Ingot"), 1, {6}},
          {filterLabel("Manasteel Ingot"), 1, {7}},
          {filterLabel("Draconium Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotC
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotC", "east", "5a8", Actions::north, Actions::south,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotC"), 1}}, {
          {filterLabel("Blutonium Ingot"), 1, {0}},
          {filterLabel("Ludicrite Ingot"), 1, {1}},
          {filterLabel("Enchanted Ingot"), 1, {2}},
          {filterLabel("Glowstone Ingot"), 1, {3}},
          {filterLabel("Magnesium Ingot"), 1, {4}},
          {filterLabel("Beryllium Ingot"), 1, {5}},
          {filterLabel("Zirconium Ingot"), 1, {6}},
          {filterLabel("Electrum Ingot"), 1, {7}},
          {filterLabel("Graphite Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotD
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotD", "west", "5b0", Actions::up, Actions::down,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotD"), 1}}, {
          {filterLabel("Platinum Ingot"), 1, {0}},
          {filterLabel("Aluminum Ingot"), 1, {1}},
          {filterLabel("Thorium Ingot"), 1, {2}},
          {filterLabel("Cyanite Ingot"), 1, {3}},
          {filterLabel("Uranium Ingot"), 1, {4}},
          {filterLabel("Iridium Ingot"), 1, {5}},
          {filterLabel("Alumite Ingot"), 1, {6}},
          {filterLabel("Lithium Ingot"), 1, {7}},
          {filterLabel("Carbon Brick"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotE
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotE", "west", "5b0", Actions::east, Actions::down,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotE"), 1}}, {
          {filterLabel("Osmium Ingot"), 1, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}},
          {filterLabel("Steel Ingot"), 1, {2}},
          {filterLabel("Tough Alloy"), 1, {3}},
          {filterLabel("Demon Ingot"), 1, {4}},
          {filterLabel("Boron Ingot"), 1, {5}},
          {filterLabel("Lead Ingot"), 1, {6}},
          {filterLabel("Iron Ingot"), 1, {7}},
          {filterLabel("Gold Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotF
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotE", "west", "5b0", Actions::north, Actions::down,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotF"), 1}}, {
          {filterLabel("Fluxed Electrum Ingot"), 1, {0}},
          {filterLabel("Ebony Psimetal Ingot"), 1, {1}},
          {filterLabel("Ivory Psimetal Ingot"), 1, {2}},
          {filterLabel("Osmiridium Ingot"), 1, {3}},
          {filterLabel("Osgloglas Ingot"), 1, {4}},
          {filterLabel("Psimetal Ingot"), 1, {5}},
          {filterLabel("Ironwood Ingot"), 1, {6}},
          {filterLabel("Mirion Ingot"), 1, {7}},
          {filterLabel("Fiery Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotG
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotG", "west", "5b0", Actions::west, Actions::down,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotG"), 1}}, {
          {filterLabel("Knightmetal Ingot"), 1, {0}},
          {filterLabel("Constantan Ingot"), 1, {5}},
          {filterLabel("Enderium Ingot"), 1, {8}},
          {filterLabel("Signalum Ingot"), 1, {6}},
          {filterLabel("Copper Ingot"), 1, {1}},
          {filterLabel("Bronze Ingot"), 1, {4}},
          {filterLabel("Lumium Ingot"), 1, {7}},
          {filterLabel("Invar Ingot"), 1, {3}},
          {filterLabel("Tin Ingot"), 1, {2}}
        }, 1}
      }));

    // ultimateIngotH
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotH", "west", "625", Actions::west, Actions::up,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotH"), 1}}, {
          {filterLabel("Aluminum Brass Ingot"), 1, {0}},
          {filterLabel("Knightslime Ingot"), 1, {1}},
          {filterLabel("Manyullyn Ingot"), 1, {2}},
          {filterLabel("Titanium Ingot"), 1, {3}},
          {filterLabel("Pigiron Ingot"), 1, {4}},
          {filterLabel("Silicon Ingot"), 1, {5}},
          {filterLabel("Cobalt Ingot"), 1, {6}},
          {filterLabel("Ardite Ingot"), 1, {7}},
          {filterLabel("Ender Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngotI
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngotI", "west", "625", Actions::north, Actions::up,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("ultimateIngotI"), 1}}, {
          {filterLabel("Electrical Steel Ingot"), 1, {0}},
          {filterLabel("Alchemical Brass Ingot"), 1, {1}},
          {filterLabel("Enhanced Ender Ingot"), 1, {2}},
          {filterLabel("Vibrant Alloy Ingot"), 1, {3}},
          {filterLabel(u8"§5Insanium Ingot"), 1, {4}},
          {filterLabel("Void Metal Ingot"), 1, {5}},
          {filterLabel("End Steel Ingot"), 1, {6}},
          {filterLabel("Soularium Ingot"), 1, {7}},
          {filterLabel("Thaumium Ingot"), 1, {8}}
        }, 1}
      }));

    // ultimateIngot
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ultimateIngot", "west", "625", Actions::east, Actions::up,
      std::vector<size_t>{
        0, 1, 2, 3, 4, 5, 6, 7, 8
      }, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("The Ultimate Ingot"), 16}}, {
          {filterLabel("ultimateIngotA"), 1, {0}},
          {filterLabel("ultimateIngotB"), 1, {1}},
          {filterLabel("ultimateIngotC"), 1, {2}},
          {filterLabel("ultimateIngotD"), 1, {3}},
          {filterLabel("ultimateIngotE"), 1, {4}},
          {filterLabel("ultimateIngotF"), 1, {5}},
          {filterLabel("ultimateIngotG"), 1, {6}},
          {filterLabel("ultimateIngotH"), 1, {7}},
          {filterLabel("ultimateIngotI"), 1, {8}}
        }, 16}
      }));

    // rockCrusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCrusher", "south", "547", Actions::up, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Zirconium Dust"), 16},
          {filterLabel("Crushed Fluorite"), 16}
        }, {{filterLabel("Diorite"), 1, {0}}}, 16},
        {{
          {filterLabel("Beryllium Dust"), 16}
        }, {{filterLabel("Andesite"), 1, {0}}}, 16},
        {{
          {filterLabel("Crushed Rhodochrosite"), 16}
        }, {{filterLabel("Granite"), 1, {0}}}, 16}
      }));

    // dragonHeart
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "dragonHeart", "east", "c77", Actions::up,
      ProcessRedstoneEmitter::makeNeeded(factory, "dragonHeart", filterLabel("Dragon Heart"), 16)));

    // porkChop
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "porkChop", "east", "c77", Actions::west,
      ProcessRedstoneEmitter::makeNeeded(factory, "porkChop", filterLabel("Raw Porkchop"), 16)));

    // cokeOven
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "cokeOven", "center", "f96", Actions::north,
      ProcessRedstoneEmitter::makeNeeded(factory, "cokeOven", filterLabel("Coal Coke"), 16)));

    // blueSlime
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "blueSlime", "east", "570", Actions::south,
      ProcessRedstoneEmitter::makeNeeded(factory, "blueSlime", blueSlime, 16)));

    // primePeacock
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primePeacock", "east", "570", Actions::east,
      ProcessRedstoneEmitter::makeNeeded(factory, "primePeacock", filterLabel("Raw Prime Peacock"), 16)));

    // peacock
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "peacock", "east", "f98", Actions::up,
      ProcessRedstoneEmitter::makeNeeded(factory, "peacock", filterLabel("Raw Peacock"), 16)));

    // primePork
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primePork", "east", "f98", Actions::west,
      ProcessRedstoneEmitter::makeNeeded(factory, "primePork", filterLabel("Raw Prime Pork"), 16)));

    // frogLegs
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "frogLegs", "east", "2fe", Actions::up,
      ProcessRedstoneEmitter::makeNeeded(factory, "frogLegs", filterLabel("Raw Frog Legs"), 16)));

    // chevon
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "chevon", "east", "f98", Actions::south,
      ProcessRedstoneEmitter::makeNeeded(factory, "chevon", filterLabel("Raw Chevon"), 16)));

    // primeChevon
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primeChevon", "east", "570", Actions::up,
      ProcessRedstoneEmitter::makeNeeded(factory, "chevon", filterLabel("Raw Prime Chevon"), 16)));

    // primeMutton
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primeMutton", "east", "78c", Actions::south,
      ProcessRedstoneEmitter::makeNeeded(factory, "primeMutton", filterLabel("Raw Prime Mutton"), 16)));

    // primeRabbit
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primeRabbit", "east", "78c", Actions::west,
      ProcessRedstoneEmitter::makeNeeded(factory, "primeRabbit", filterLabel("Raw Prime Rabbit"), 16)));

    // horse
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "horse", "east", "78c", Actions::up,
      ProcessRedstoneEmitter::makeNeeded(factory, "horse", filterLabel("Raw Horse"), 16)));

    // witherSkeleton
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "witherSkeleton", "east", "570", Actions::north,
      [&]() {
        if (factory.getAvail(factory.getItem(ItemFilters::Label("Bone")), true) < 16
            || factory.getAvail(factory.getItem(ItemFilters::Label("Wither Ash")), true) < 16
            || factory.getAvail(factory.getItem(ItemFilters::Label("Wither Skeleton Skull")), true) < 63) {
          factory.log("witherSkeleton: on", 0xff4fff);
          return 15;
        } else {
          return 0;
        }
      }));

    // primeBeef
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primeBeef", "east", "2fe", Actions::east,
      [&]() {
        if (factory.getAvail(factory.getItem(ItemFilters::Label("Raw Prime Beef")), true) < 16
            || factory.getAvail(factory.getItem(ItemFilters::Label("Leather")), true) < 16) {
          factory.log("primeBeef: on", 0xff4fff);
          return 15;
        } else {
          return 0;
        }
      }));

    // primeChicken
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "primeChicken", "east", "f98", Actions::north,
      [&]() {
        if (factory.getAvail(factory.getItem(ItemFilters::Label("Raw Prime Chicken")), true) < 16
            || factory.getAvail(factory.getItem(ItemFilters::Label("Feather")), true) < 16) {
          factory.log("primeChicken: on", 0xff4fff);
          return 15;
        } else {
          return 0;
        }
      }));

    // shulker
    factory.addProcess(std::make_unique<ProcessRedstoneEmitter>(factory, "shulker", "east", "2fe", Actions::south,
      ProcessRedstoneEmitter::makeNeeded(factory, "shulker", filterLabel("Shulker Shell"), 16)));

    // fusion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "fusion", "south", "00a", Actions::east, Actions::up,
      std::vector<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21},
      nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
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
          {filterLabelName("Block of Black Iron", "extendedcrafting:storage"), 1, {3}},
          {filterLabel("Ink Sac"), 1, {4}}
        }, 4},
        {{{filterLabel("Hardened Cell Frame"), 4}}, {
          {filterLabel("Energy Cell Frame"), 1, {0}},
          {filterLabel("Invar Plate"), 1, {1}},
          {filterLabel("Steel Rod"), 1, {2}},
          {filterLabel("Steel Casing"), 1, {3}},
          {filterLabel("Invar Gear"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Palis Crystal Block"), 16}}, {
          {filterLabel("Palis Crystal Block"), 1, {0}},
          {filterLabel("Dense Lapis Lazuli Plate"), 1, {1}},
          {filterLabel("Cobalt Ingot"), 1, {2}},
          {filterLabel("Sapphire"), 1, {3}},
          {congealedBlueSlime, 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Diamatine Crystal Block"), 16}}, {
          {filterLabel("Diamatine Crystal Block"), 1, {0}},
          {filterLabel("Mana Diamond"), 1, {1}},
          {filterLabel("Malachite"), 1, {2}},
          {filterLabel("Manyullyn Ingot"), 1, {3}},
          {filterLabel("Zirconium Dust"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Enori Crystal Block"), 16}}, {
          {filterLabel("Enori Crystal Block"), 1, {0}},
          {filterLabel("Bone Block"), 1, {1}},
          {filterLabel("Block of Quartz"), 1, {2}},
          {filterLabel("Osmium Ingot"), 1, {3}},
          {filterLabel("Fluorite"), 1, {4}}
        }, 4},
        {{{filterLabel("Empowered Emeradic Crystal Block"), 16}}, {
          {filterLabel("Emeradic Crystal Block"), 1, {0}},
          {filterLabel("Cactus Green"), 1, {1}},
          {filterLabel("Ethetic Green Block"), 1, {2}},
          {filterLabel("Beryllium Dust"), 1, {3}},
          {filterLabel("Emerald"), 1, {4}}
        }, 4},
        {{{filterLabel("Resonant Cell Frame (Full)"), 2}}, {
          {filterLabel("Moon Stone"), 1, {0}},
          {filterLabel("Dense Steel Plate"), 1, {1}},
          {filterLabel("Lapotron Crystal"), 1, {2}},
          {filterLabel("Ender Electron Tube"), 1, {3}},
          {filterLabel("Pulsating Mesh"), 1, {4}},
          {filterLabel("Reactor Frame"), 1, {5}},
          {filterLabel("Structure Frame Tier 1"), 1, {6}},
          {filterLabel("Infused Diamond"), 1, {7}},
          {filterLabel("Ultimate Control Circuit"), 1, {8}},
          {filterLabel("Enderium Gear"), 1, {9}},
          {filterLabel("Litherite Crystal"), 1, {10}},
          {filterLabel("Bioplastic"), 1, {11}},
          {filterLabel("Cubic Boron Nitride"), 1, {12}},
          {filterLabel("Genetics Processor"), 1, {13}},
          {filterLabel("Resonant Cell Frame (Empty)"), 1, {14}}
        }, 4},
        {{{filterLabel("Extreme Crafting Table"), 4}}, {
          {filterLabel("Crystal Matrix Ingot"), 4, {0}},
          {filterLabel("Crystaltine Catalyst"), 2, {1}},
          {filterLabel("Double Compressed Crafting Table"), 1, {2}}
        }, 16},
        {{{filterLabel("Charged Draconium Block"), 4}}, {
          {filterLabel("Draconium Block"), 2, {0}}
        }, 8}
      }));

    // draconicFusion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "draconicFusion", "west", "9d8", Actions::north, Actions::east,
      std::vector<size_t>{0, 1, 2, 3, 4, 5},
      nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Awakened Draconium Block"), 16}}, {
          {filterLabel("Charged Draconium Block"), 4, {0}},
          {filterLabel("Grains of Infinity"), 1, {1}},
          {filterLabel("Grains of the End"), 1, {2}},
          {filterLabel("Draconic Core"), 4, {3}},
          {filterLabel("Dragon Heart"), 1, {4}},
          {filterLabel("Dragonstone"), 1, {5}}
        }, 16},
        {{{filterLabel("Awakened Core"), 16}}, {
          {filterLabel("Wyvern Core"), 4, {0}},
          {filterLabel("Awakened Draconium Ingot"), 5, {1}},
          {filterLabel("Nether Star"), 1, {2}}
        }, 15}
      }));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "center", "1dc", "ffd",
      Actions::south, Actions::south, Actions::up,
      std::vector<Recipe<std::pair<int, std::vector<NonConsumableInfo>>, std::vector<size_t>>>{
        {{}, {{filterLabel("Dragon Egg Chunk"), 3, {1, 2, 4}}}, {21, {}}},
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
          {filterLabel("Terra Vis Crystal"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Vibrant Crystal"), 16}}, {
          {filterLabel("Vibrant Alloy Nugget"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Emerald"), 1, {5}}
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
        {{{filterLabel("Raw Carbon Fibre"), 8}}, {
          {filterLabel("Pulverized Coal"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Raw Carbon Mesh"), 8}}, {
          {filterLabel("Raw Carbon Fibre"), 2, {1, 4}}
        }, {32, {}}},
        {{{filterLabel("Certus Quartz Seed"), 16}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Certus Quartz Dust"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Nether Quartz Seed"), 16}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Nether Quartz Dust"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Fluix Seed"), 16}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Fluix Dust"), 1, {2}}
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
        {{{filterLabel("Machine Case"), 8}}, {
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
        {{{filterLabel("Dark Iron Bars"), 16}}, {
          {filterLabel("Dark Steel Ingot"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("Glass Pane"), 16}}, {
          {filterLabel("Glass"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("String"), 16}}, {
          {filterLabel("Flax"), 3, {1, 2, 4}}
        }, {21, {}}},
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
        {{{filterLabel("Range Addon"), 4}}, {
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
        {{{filterLabel("Basic Plating"), 16}}, {
          {filterLabel("Lead Sheetmetal"), 4, {1, 3, 7, 9}},
          {filterLabel("Lead Item Casing"), 4, {2, 4, 6, 8}},
          {filterLabel("Graphite Block"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Advanced Plating"), 16}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Tough Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Basic Plating"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("DU Plating"), 16}}, {
          {filterLabel("Sulfur"), 4, {1, 3, 7, 9}},
          {filterLabel("Uranium-238"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Plating"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Elite Plating"), 16}}, {
          {filterLabel("Crystal Binder"), 4, {1, 3, 7, 9}},
          {filterLabel("Boron Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("DU Plating"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Crystal Binder"), 16}}, {
          {filterLabel("Crushed Rhodochrosite"), 1, {1}},
          {filterLabel("Calcium Sulfate"), 1, {2}},
          {filterLabel("Pulverized Obsidian"), 1, {4}},
          {filterLabel("Magnesium Dust"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Plain Yogurt"), 16}}, {
          {filterLabel("Soy Milk"), 1, {1}}
        }, {64, {{1, 9}}}},
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
        {{{filterLabel("Thankful Dinner"), 64}}, {
          {filterLabel("Cooked Tofurkey"), 1, {1}},
          {filterLabel("Mashed Potatoes"), 1, {2}},
          {filterLabel("Sweet Potato Pie"), 1, {3}},
          {filterLabel("Cranberry Jelly"), 1, {4}},
          {filterLabel("Corn"), 1, {5}},
          {filterLabel("Onion"), 1, {6}},
          {filterLabel("Toast"), 1, {7}}
        }, {12, {{2, 9}}}},
        {{{filterLabel("Mashed Potatoes"), 16}}, {
          {filterLabel("Buttered Potato"), 1, {1}},
          {filterLabel("Salt"), 1, {2}}
        }, {64, {{4, 9}}}},
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
        {{{filterLabel("Nether Star Nugget"), 16}}, {
          {filterLabel("Nether Star"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Crystaltine Nugget"), 16}}, {
          {filterLabel("Crystaltine Ingot"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Vibrant Alloy Nugget"), 16}}, {
          {filterLabel("Vibrant Alloy Ingot"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Blutonium Ingot"), 16}}, {
          {filterLabel("Blutonium Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Awakened Draconium Ingot"), 16}}, {
          {filterLabel("Awakened Draconium Block"), 1, {5}}
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
        {{{filterLabel("Ludicrite Ingot"), 16}}, {
          {filterLabel("Ludicrite Block"), 1, {5}}
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
        {{{filterLabel("Soul Vial"), 16}}, {
          {filterLabel("Fused Quartz"), 3, {4, 6, 8}},
          {filterLabel("Soularium Ingot"), 1, {2}}
        }, {16, {}}},
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
        {{{filterLabel("Gunpowder"), 64}}, {
          {filterLabel("Creeper Essence"), 3, {1, 2, 3}}
        }, {10, {}}},
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
        {{{filterLabel("Aluminium Scaffolding"), 8}}, {
          {filterLabel("Aluminum Ingot"), 3, {1, 2, 3}},
          {filterLabel("Aluminium Rod"), 3, {5, 7, 9}}
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
        {{{filterLabel("Steel Casing"), 4}}, {
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
        {{{filterLabel("Heavy Engineering Block"), 8}}, {
          {filterLabel("Uranium Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Mechanical Component"), 2, {4, 6}},
          {filterLabel("Reinforced Alloy"), 2, {2, 8}},
          {filterLabel("Steel Scaffolding"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Light Engineering Block"), 8}}, {
          {filterLabel("Bronze Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Enriched Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Aluminium Scaffolding"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Engineering Block"), 8}}, {
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
        {{{filterLabel("Redstone Servo"), 8}}, {
          {filterLabel("Redstone"), 2, {2, 8}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Pyrotheum Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {3}},
          {filterLabel("Sulfur"), 1, {4}}
        }, {32, {}}},
        {{{filterLabel("Petrotheum Dust"), 16}}, {
          {filterLabel("Basalz Powder"), 2, {1, 2}},
          {filterLabel("Pulverized Obsidian"), 1, {3}},
          {filterLabel("Redstone"), 1, {4}}
        }, {32, {}}},
        {{{filterLabel("Aerotheum Dust"), 16}}, {
          {filterLabel("Blitz Powder"), 2, {1, 2}},
          {filterLabel("Niter"), 1, {3}},
          {filterLabel("Redstone"), 1, {4}}
        }, {32, {}}},
        {{{filterLabel("Lapis Lazuli"), 64}}, {
          {filterLabel("Lapis Lazuli Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
        {{{filterLabel("Fiery Ingot"), 64}}, {
          {filterLabel("Fiery Ingot Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Iron Ingot"), 64}}, {
          {filterLabel("Iron Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Coal"), 64}}, {
          {filterLabel("Coal Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
        {{{filterLabel("Void Metal Ingot"), 64}}, {
          {filterLabel("Void Metal Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Thaumium Ingot"), 64}}, {
          {filterLabel("Thaumium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Ironwood Ingot"), 64}}, {
          {filterLabel("Ironwood Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Steeleaf"), 64}}, {
          {filterLabel("Steeleaf Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Copper Ingot"), 64}}, {
          {filterLabel("Copper Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Knightmetal Ingot"), 64}}, {
          {filterLabel("Knightmetal Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Knightslime Ingot"), 64}}, {
          {filterLabel("Knightslime Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Sky Stone"), 16}}, {
          {filterLabel("Sky Stone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
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
        {{{filterLabel("Lime Dye"), 16}}, {
          {filterLabel("Dye Essence"), 3, {3, 5, 7}}
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
        {{{filterLabel("Granite"), 16}}, {
          {filterLabel("Diorite"), 1, {1}},
          {filterLabel("Nether Quartz"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Diorite"), 16}}, {
          {filterLabel("Cobblestone"), 2, {1, 5}},
          {filterLabel("Nether Quartz"), 2, {2, 4}}
        }, {32, {}}},
        {{{filterLabel("Andesite"), 16}}, {
          {filterLabel("Diorite"), 1, {1}},
          {filterLabel("Cobblestone"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Quartz Slab"), 16}}, {
          {filterLabel("Block of Quartz"), 3, {4, 5, 6}}
        }, {10, {}}},
        {{{filterLabel("Chiseled Quartz Block"), 16}}, {
          {filterLabel("Quartz Slab"), 2, {2, 5}}
        }, {32, {}}},
        {{{filterLabel("LV Capacitor"), 8}}, {
          {filterLabel("Iron Ingot"), 3, {1, 2, 3}},
          {filterLabel("Copper Ingot"), 2, {4, 6}},
          {filterLabel("Treated Wood Planks"), 2, {7, 9}},
          {filterLabel("Lead Ingot"), 1, {5}},
          {filterLabel("Redstone"), 1, {8}}
        }, {21, {}}},
        {{{filterLabel("MV Capacitor"), 8}}, {
          {filterLabel("Iron Ingot"), 3, {1, 2, 3}},
          {filterLabel("Electrum Ingot"), 2, {4, 6}},
          {filterLabel("Treated Wood Planks"), 2, {7, 9}},
          {filterLabel("LV Capacitor"), 1, {5}},
          {filterLabel("Block of Redstone"), 1, {8}}
        }, {21, {}}},
        {{{filterLabel("HV Capacitor"), 8}}, {
          {filterLabel("Steel Ingot"), 3, {1, 2, 3}},
          {filterLabel("Block of Lead"), 2, {4, 6}},
          {filterLabel("Treated Wood Planks"), 2, {7, 9}},
          {filterLabel("MV Capacitor"), 1, {5}},
          {filterLabel("Block of Redstone"), 1, {8}}
        }, {21, {}}},
        {{{filterLabel("Basic Capacitor"), 8}}, {
          {filterLabel("Redstone Transmission Coil"), 4, {2, 4, 6, 8}},
          {filterLabel("Grains of Infinity"), 2, {3, 7}},
          {filterLabel("HV Capacitor"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Double-Layer Capacitor"), 8}}, {
          {filterLabel("Basic Capacitor"), 2, {4, 6}},
          {filterLabel("Energetic Alloy Ingot"), 2, {2, 8}},
          {filterLabel("Coke Dust"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Octadic Capacitor"), 8}}, {
          {filterLabel("Double-Layer Capacitor"), 2, {4, 6}},
          {filterLabel("Vibrant Alloy Ingot"), 2, {2, 8}},
          {filterLabel("Ferroboron Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Charged Draconium Block"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Conduit Binder Composite"), 16}}, {
          {filterLabel("Gravel"), 5, {1, 3, 5, 7, 9}},
          {filterName("minecraft:clay_ball"), 2, {2, 8}},
          {filterLabel("Sand"), 2, {4, 6}}
        }, {8, {}}},
        {{{filterLabel("Infinity Bimetal Gear"), 8}}, {
          {filterLabel("Iron Nugget"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Grains of Infinity"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Simple Machine Chassis"), 1}}, {
          {filterLabel("Titanium Aluminide Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Infinity Bimetal Gear"), 2, {2, 8}},
          {filterLabel("Dark Iron Bars"), 2, {4, 6}},
          {filterLabel("Hardened Cell Frame"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Soul Attuned Dye Blend"), 1}}, {
          {filterLabel("Soul Powder"), 2, {1, 9}},
          {filterLabel("Organic Brown Dye"), 2, {3, 7}},
          {filterLabel("Nether Quartz Dust"), 4, {2, 4, 6, 8}},
          {filterLabel("Organic Black Dye"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Industrial Dye Blend"), 1}}, {
          {filterLabel("Lapis Lazuli Dust"), 2, {1, 9}},
          {filterLabel("Organic Green Dye"), 2, {3, 7}},
          {filterLabel("Nether Quartz Dust"), 4, {2, 4, 6, 8}},
          {filterLabel("Organic Black Dye"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Quartz Glass"), 16}}, {
          {filterLabel("Nether Quartz Dust"), 5, {1, 3, 5, 7, 9}},
          {filterLabel("Glass"), 4, {2, 4, 6, 8}}
        }, {12, {}}},
        {{{filterLabel("Quartz Fiber"), 16}}, {
          {filterLabel("Nether Quartz Dust"), 3, {4, 5, 6}},
          {filterLabel("Glass"), 6, {1, 2, 3, 7, 8, 9}}
        }, {10, {}}},
        {{{filterLabel("ME Glass Cable - Fluix"), 16}}, {
          {filterLabel("Quartz Fiber"), 1, {1}},
          {filterLabel("Pure Fluix Crystal"), 2, {2, 4}}
        }, {16, {}}},
        {{{filterLabel("Iridium Reinforced Plate"), 16}}, {
          {filterLabel("Iridium Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Advanced Alloy"), 4, {2, 4, 6, 8}},
          {filterLabel("Diamond"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Connector"), 16}}, {
          {filterLabel("Signalum Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Block of Tin"), 4, {2, 4, 6, 8}},
          {filterLabel("Aluminum Brass Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Black Concrete Powder"), 16}}, {
          {filterLabel("Sand"), 4, {1, 3, 7, 9}},
          {filterLabel("Gravel"), 4, {2, 4, 6, 8}},
          {filterLabel("Ink Sac"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Interconnect"), 16}}, {
          {filterLabel("Connector"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Concrete"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Black Iron Slate"), 16}}, {
          {filterLabel("Black Iron Ingot"), 2, {4, 5}}
        }, {32, {}}},
        {{{filterLabel("Mana Dust"), 16}}, {
          {filterLabel("Mana Diamond"), 4, {1, 3, 7, 9}},
          {filterLabel("Mana Powder"), 1, {5}},
          {filterLabel("Cryotheum Dust"), 1, {2}},
          {filterLabel("Pyrotheum Dust"), 1, {4}},
          {filterLabel("Aerotheum Dust"), 1, {6}},
          {filterLabel("Petrotheum Dust"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("1k ME Fluid Storage Component"), 3}}, {
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}},
          {filterLabel("Pure Certus Quartz Crystal"), 4, {2, 4, 6, 8}},
          {filterLabel("Logic Processor"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("4k ME Fluid Storage Component"), 3}}, {
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}},
          {filterLabel("1k ME Fluid Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("16k ME Fluid Storage Component"), 3}}, {
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}},
          {filterLabel("4k ME Fluid Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("64k ME Fluid Storage Component"), 3}}, {
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}},
          {filterLabel("16k ME Fluid Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("256k ME Fluid Storage Component"), 1}}, {
          {filterLabel("Lapis Lazuli"), 4, {1, 3, 7, 9}},
          {filterLabel("64k ME Fluid Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("Engineering Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("Fluix Pearl"), 16}}, {
          {filterLabel("Fluix Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Pure Fluix Crystal"), 4, {2, 4, 6, 8}},
          {filterLabel("Ender Pearl"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Lever"), 16}}, {
          {filterLabel("Stick"), 1, {2}},
          {filterLabel("Cobblestone"), 1, {5}}
        }, {64, {}}},
        {{{filterLabel("Ender Ingot"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {1}},
          {filterLabel("Ender Pearl"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Analog Crafter"), 8}}, {
          {filterLabel("Crafting Table"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Lever"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Copper Solenoid"), 16}}, {
          {filterLabel("Copper Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Mixed Metal Ingot"), 1, {5}},
          {filterLabel("Copper Item Casing"), 2, {2, 8}},
          {filterLabel("Aluminium Rod"), 2, {4, 6}}
        }, {16, {}}},
        {{{filterLabel("Insulating Glass"), 16}}, {
          {filterLabel("Pulverized Iron"), 2, {4, 6}},
          {filterLabel("Cactus Green"), 1, {5}},
          {filterLabel("Glass"), 2, {2, 8}}
        }, {32, {}}},
        {{{filterLabel("Charged Quartz Fixture"), 16}}, {
          {filterLabel("Charged Certus Quartz Crystal"), 1, {4}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Energy Acceptor"), 1}}, {
          {filterLabel("Vacuum Tube"), 2, {1, 3}},
          {filterLabel("Redstone Engineering Block"), 1, {2}},
          {filterLabel("ME Glass Cable - Fluix"), 2, {4, 6}},
          {filterLabel("Charged Quartz Fixture"), 1, {5}},
          {filterLabel("Fluix Block"), 2, {7, 9}},
          {filterLabel("Engineering Processor"), 1, {8}}
        }, {32, {}}},
        {{{filterLabel("User Interface"), 8}}, {
          {filterLabel("Lime Dye"), 2, {4, 6}},
          {filterLabel("Glowstone Dust"), 2, {7, 9}},
          {filterLabel("Redstone"), 1, {5}},
          {filterLabel("Glass Pane"), 1, {8}}
        }, {32, {}}},
        {{{filterLabel("Control Circuit Board"), 8}}, {
          {filterLabel("Elite Control Circuit"), 4, {1, 3, 7, 9}},
          {filterLabel("Bioplastic"), 2, {4, 6}},
          {filterLabel("Circuit Board"), 1, {5}},
          {filterLabel("Copper Coil Block"), 2, {2, 8}}
        }, {16, {}}},
        {{{filterLabel("Item IO Circuit Board"), 8}}, {
          {filterLabel("Printed Logic Circuit"), 6, {1, 4, 7, 3, 6, 9}},
          {filterLabel("Enhanced Circuit Board"), 2, {2, 8}},
          {filterLabel("Circuit Board"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("ME Controller"), 1}}, {
          {filterLabel("Sky Stone Block"), 2, {1, 3}},
          {filterLabel("Fluix Block"), 2, {4, 6}},
          {filterLabel("Machine Case"), 2, {7, 9}},
          {filterLabel("Energy Acceptor"), 1, {5}},
          {filterLabel("Control Circuit Board"), 1, {2}},
          {filterLabel("Fluix Pearl"), 1, {8}}
        }, {32, {}}},
        {{{filterLabel("Machine Block"), 1}}, {
          {filterLabel("Polished Stone"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Electron Tube"), 4, {2, 4, 6, 8}},
          {filterLabel("Sturdy Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Crafter Tier 1"), 1}}, {
          {filterLabel("Redstone Gear"), 2, {2, 8}},
          {filterLabel("Analog Crafter"), 2, {4, 6}},
          {filterLabel("Machine Block"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Crafter Tier 2"), 1}}, {
          {filterLabel("Redstone Gear"), 2, {2, 8}},
          {filterLabel("Analog Crafter"), 2, {4, 6}},
          {filterLabel("Crafter Tier 1"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Crafter Tier 3"), 1}}, {
          {filterLabel("Redstone Gear"), 2, {2, 8}},
          {filterLabel("Analog Crafter"), 2, {4, 6}},
          {filterLabel("Crafter Tier 2"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Annihilation Core"), 1}}, {
          {filterLabel("Pure Nether Quartz Crystal"), 1, {4}},
          {filterLabel("Fluix Dust"), 1, {5}},
          {filterLabel("Logic Processor"), 1, {6}}
        }, {32, {}}},
        {{{filterLabel("Formation Core"), 1}}, {
          {filterLabel("Pure Certus Quartz Crystal"), 1, {4}},
          {filterLabel("Fluix Dust"), 1, {5}},
          {filterLabel("Logic Processor"), 1, {6}}
        }, {32, {}}},
        {{{filterLabel("Iron Frame"), 16}}, {
          {filterLabel("Iron Rod"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Bars"), 4, {2, 4, 6, 8}},
          {filterLabel("Iron Mechanical Component"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone-Iron Wiring"), 16}}, {
          {filterLabel("Basic Coil"), 3, {3, 5, 7}}
        }, {16, {{6, 2}, {7, 4}, {8, 6}, {9, 8}}}},
        {{{filterLabel("Iron Tubing"), 16}}, {
          {filterLabel("Basic Coil"), 3, {3, 5, 7}},
          {filterLabel("Aluminium Wire"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Hardened Upgrade Kit"), 8}}, {
          {filterLabel("Invar Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone"), 2, {7, 9}},
          {filterLabel("Bronze Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Reinforced Upgrade Kit"), 8}}, {
          {filterLabel("Electrum Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Hardened Glass"), 2, {7, 9}},
          {filterLabel("Silver Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Signalum Upgrade Kit"), 8}}, {
          {filterLabel("Signalum Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Cryotheum Dust"), 2, {7, 9}},
          {filterLabel("Electrum Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Resonant Upgrade Kit"), 8}}, {
          {filterLabel("Enderium Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Pyrotheum Dust"), 2, {7, 9}},
          {filterLabel("Lumium Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Enriched Uranium Nuclear Fuel"), 16}}, {
          {filterLabel("Uranium Ingot"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Tiny Clump of Uranium-235"), 3, {4, 5, 6}}
        }, {10, {}}},
        {{{filterLabel("Blutonium Block"), 16}}, {
          {filterLabel("Block of Mana Infused Metal"), 2, {1, 3}},
          {filterLabel("Block of Cobalt"), 1, {2}},
          {filterLabel("Tiny Pile of Plutonium"), 3, {4, 5, 6}},
          {filterLabel("Cyanite Block"), 2, {7, 9}},
          {filterLabel("Empowered Palis Crystal Block"), 1, {8}}
        }, {21, {}}},
        {{{filterLabel("Genetics Processor"), 16}}, {
          {filterLabel("Printed Engineering Circuit"), 4, {1, 3, 7, 9}},
          {filterLabel("Pure Nether Quartz Crystal"), 2, {2, 8}},
          {filterLabel("Advanced Control Circuit"), 2, {4, 6}},
          {filterLabel("Ender Pearl"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Eye of Ender"), 16}}, {
          {filterLabel("Ender Pearl"), 1, {1}},
          {filterLabel("Blaze Powder"), 1, {2}}
        }, {64, {}}},
        {{{filterLabel("Redstone Alloy Grinding Ball"), 16}}, {
          {filterLabel("Redstone Alloy Ingot"), 5, {2, 4, 5, 6, 8}}
        }, {2, {}}},
        {{{filterLabel("Genetics Labware"), 16}}, {
          {filterLabel("Glass Pane"), 4, {1, 3, 4, 6}},
          {filterLabel("Diamond"), 1, {8}}
        }, {4, {}}},
        {{{filterLabel("Terrestrial Artifact"), 16}}, {
          {filterLabel("Empowered Restonia Crystal"), 4, {1, 3, 7, 9}},
          {filterLabel("Empowered Emeradic Crystal"), 1, {2}},
          {filterLabel("Empowered Palis Crystal"), 1, {4}},
          {filterLabel("Empowered Diamatine Crystal"), 1, {5}},
          {filterLabel("Empowered Void Crystal"), 1, {6}},
          {filterLabel("Empowered Enori Crystal"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Ender Amethyst"), 16}}, {
          {filterLabel("Elementium Ingot"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Terrestrial Artifact"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Ender Casing"), 8}}, {
          {filterLabel("Ender Pearl"), 4, {1, 3, 7, 9}},
          {filterLabel("Empowered Diamatine Crystal"), 4, {2, 4, 6, 8}},
          {filterLabel("Block of Black Quartz"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Pulsating Mesh"), 8}}, {
          {filterLabel("Pulsating Propolis"), 5, {1, 3, 5, 7, 9}}
        }, {12, {}}},
        {{{filterLabel("Infused Diamond"), 16}}, {
          {filterLabel("Dimensional Shard"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Diamond"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Stable-'Unstable Nugget'"), 16}}, {
          {filterLabel("Iron Nugget"), 1, {2}},
          {filterLabel("Stick"), 1, {5}},
          {filterLabel("Diamond"), 1, {8}}
        }, {64, {}}},
        {{{filterLabel("Moon Stone"), 16}}, {
          {filterLabel("Lunar Reactive Dust"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Stable-'Unstable Ingot'"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Reactor Frame"), 8}}, {
          {filterLabel("Steel Casing"), 4, {2, 4, 6, 8}},
          {filterLabel("Atomic Alloy"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Lapotron Crystal"), 2}}, {
          {filterLabel("Energy Crystal"), 1, {5}},
          {filterLabel("Lapis Lazuli Dust"), 6, {1, 3, 4, 6, 7, 9}},
          {filterLabel("Advanced Control Circuit"), 2, {2, 8}}
        }, {2, {}}},
        {{{filterLabel("Structure Frame Tier 1"), 8}}, {
          {filterLabel("Interconnect"), 1, {5}},
          {filterLabel("Litherite Crystal"), 2, {4, 6}},
          {filterLabel("Iron Ingot"), 1, {2}},
          {filterLabel("Lapis Lazuli"), 1, {8}}
        }, {32, {}}},
        {{{filterLabel("Disk Platter"), 16}}, {
          {filterLabel("Iron Nugget"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Sulfur"), 16}}, {
          {filterLabel("Sulfur Essence"), 3, {1, 2, 4}}
        }, {8, {}}},
        {{{filterLabel("Basic Energy Cube"), 1}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Ingot"), 2, {4, 6}},
          {filterLabel("Steel Casing"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {2}},
          {filterLabel("Energy Tablet"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Advanced Energy Cube"), 1}}, {
          {filterLabel("Enriched Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Osmium Ingot"), 2, {4, 6}},
          {filterLabel("Basic Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {2}},
          {filterLabel("Energy Tablet"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Elite Energy Cube"), 1}}, {
          {filterLabel("Reinforced Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Gold Ingot"), 2, {4, 6}},
          {filterLabel("Advanced Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {2}},
          {filterLabel("Energy Tablet"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Ultimate Energy Cube"), 1}}, {
          {filterLabel("Atomic Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Diamond"), 2, {4, 6}},
          {filterLabel("Elite Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {2}},
          {filterLabel("Energy Tablet"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Basic Induction Cell"), 4}}, {
          {filterLabel("Lithium Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Basic Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {2}},
          {filterLabel("Energy Tablet"), 1, {4}},
          {filterLabel("Energy Tablet"), 1, {6}},
          {filterLabel("Energy Tablet"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Advanced Induction Cell"), 4}}, {
          {filterLabel("Basic Induction Cell"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {1}},
          {filterLabel("Energy Tablet"), 1, {3}},
          {filterLabel("Energy Tablet"), 1, {7}},
          {filterLabel("Energy Tablet"), 1, {9}}
        }, {1, {}}},
        {{{filterLabel("Elite Induction Cell"), 4}}, {
          {filterLabel("Advanced Induction Cell"), 4, {2, 4, 6, 8}},
          {filterLabel("Elite Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {1}},
          {filterLabel("Energy Tablet"), 1, {3}},
          {filterLabel("Energy Tablet"), 1, {7}},
          {filterLabel("Energy Tablet"), 1, {9}}
        }, {1, {}}},
        {{{filterLabel("Ultimate Induction Cell"), 1}}, {
          {filterLabel("Elite Induction Cell"), 4, {2, 4, 6, 8}},
          {filterLabel("Ultimate Energy Cube"), 1, {5}},
          {filterLabel("Energy Tablet"), 1, {1}},
          {filterLabel("Energy Tablet"), 1, {3}},
          {filterLabel("Energy Tablet"), 1, {7}},
          {filterLabel("Energy Tablet"), 1, {9}}
        }, {1, {}}},
        {{{filterLabel("Basic Induction Provider"), 4}}, {
          {filterLabel("Lithium Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Basic Control Circuit"), 4, {2, 4, 6, 8}},
          {filterLabel("Basic Energy Cube"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Advanced Induction Provider"), 4}}, {
          {filterLabel("Basic Induction Provider"), 4, {2, 4, 6, 8}},
          {filterLabel("Advanced Control Circuit"), 4, {1, 3, 7, 9}},
          {filterLabel("Advanced Energy Cube"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Elite Induction Provider"), 4}}, {
          {filterLabel("Advanced Induction Provider"), 4, {2, 4, 6, 8}},
          {filterLabel("Elite Control Circuit"), 4, {1, 3, 7, 9}},
          {filterLabel("Elite Energy Cube"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Ultimate Induction Provider"), 1}}, {
          {filterLabel("Elite Induction Provider"), 4, {2, 4, 6, 8}},
          {filterLabel("Ultimate Control Circuit"), 4, {1, 3, 7, 9}},
          {filterLabel("Ultimate Energy Cube"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Dislocator"), 1}}, {
          {filterLabel("Draconium Dust"), 4, {2, 4, 6, 8}},
          {filterLabel("Blaze Powder"), 4, {1, 3, 7, 9}},
          {filterLabel("Eye of Ender"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Draconic Core"), 16}}, {
          {filterLabel("Draconium Block"), 2, {1, 3}},
          {filterLabel("Litherite Crystal"), 2, {4, 6}},
          {filterLabel("Elite Plating"), 2, {7, 9}},
          {filterLabel("Blutonium Ingot"), 1, {2}},
          {filterLabel("Genetics Processor"), 1, {5}},
          {filterLabel("Dislocator"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Basic Component"), 16}}, {
          {filterLabel("Iron Ingot"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Advanced Component"), 16}}, {
          {filterLabel("Gold Ingot"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Elite Component"), 16}}, {
          {filterLabel("Diamond"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Ultimate Component"), 16}}, {
          {filterLabel("Emerald"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Crystaltine Component"), 16}}, {
          {filterLabel("Crystaltine Ingot"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("The Ultimate Component"), 16}}, {
          {filterLabel("The Ultimate Ingot"), 2, {4, 5}},
          {filterLabel("Black Iron Slate"), 1, {1}},
          {filterLabel("Luminessence"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Basic Catalyst"), 16}}, {
          {filterLabel("Basic Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Advanced Catalyst"), 16}}, {
          {filterLabel("Advanced Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Elite Catalyst"), 16}}, {
          {filterLabel("Elite Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Ultimate Catalyst"), 16}}, {
          {filterLabel("Ultimate Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Crystaltine Catalyst"), 16}}, {
          {filterLabel("Crystaltine Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("The Ultimate Catalyst"), 16}}, {
          {filterLabel("The Ultimate Component"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Basic Crafting Table"), 4}}, {
          {filterLabel("Basic Component"), 4, {1, 3, 7, 9}},
          {filterLabel("Crafting Table"), 2, {4, 6}},
          {filterLabel("Block of Iron"), 1, {5}},
          {filterLabel("Basic Catalyst"), 1, {2}},
          {filterLabel("Black Iron Slate"), 1, {8}}
        }, {16, {}}},
        {{{crystaltineTrimmed, 16}}, {
          {filterLabel("Crystaltine Nugget"), 4, {1, 3, 7, 9}},
          {filterLabelName("Block of Black Iron", "extendedcrafting:storage"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Diamond Lattice"), 16}}, {
          {filterLabel("Diamond"), 5, {1, 3, 5, 7, 9}}
        }, {12, {}}},
        {{{filterLabel("Crystal Matrix Ingot"), 16}}, {
          {filterLabel("Diamond Lattice"), 4, {4, 6, 7, 9}},
          {filterLabel("Nether Star"), 2, {5, 8}}
        }, {16, {}}},
        {{{filterLabel("Thick Neutron Reflector"), 16}}, {
          {filterLabel("Copper Plate"), 5, {1, 3, 5, 7, 9}},
          {filterLabel("Neutron Reflector"), 4, {2, 4, 6, 8}}
        }, {12, {}}},
        {{{filterLabel("Iridium Neutron Reflector"), 4}}, {
          {filterLabel("Dense Copper Plate"), 2, {4, 6}},
          {filterLabel("Iridium Reinforced Plate"), 1, {5}},
          {filterLabel("Thick Neutron Reflector"), 6, {1, 2, 3, 7, 8, 9}}
        }, {10, {}}},
        {{{filterLabel("Extract Speed Upgrade"), 15}}, {
          {filterLabel("Iron Ingot"), 3, {1, 2, 3}},
          {filterLabel("Electrical Steel Ingot"), 4, {4, 6, 7, 9}},
          {filterLabel("Piston"), 1, {5}},
          {filterLabel("Redstone Torch"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Carbon Brick"), 16}}, {
          {filterLabel("Charcoal"), 6, {1, 2, 3, 4, 5, 6}}
        }, {10, {}}},
        {{{filterLabel("Base Essence Ingot"), 16}}, {
          {filterLabel("Prosperity Shard"), 4, {2, 4, 6, 8}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§eInferium Ingot"), 16}}, {
          {filterLabel(u8"§eInferium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Base Essence Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§aPrudentium Ingot"), 16}}, {
          {filterLabel(u8"§aPrudentium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§eInferium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§6Intermedium Ingot"), 16}}, {
          {filterLabel(u8"§6Intermedium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§aPrudentium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§bSuperium Ingot"), 16}}, {
          {filterLabel(u8"§bSuperium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§6Intermedium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§cSupremium Ingot"), 16}}, {
          {filterLabel(u8"§cSupremium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§bSuperium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§5Insanium Ingot"), 16}}, {
          {filterLabel(u8"§5Insanium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§cSupremium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel(u8"§5Insanium Ingot"), 16}}, {
          {filterLabel(u8"§5Insanium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel(u8"§cSupremium Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Book"), 16}}, {
          {filterLabel("Paper"), 3, {1, 2, 4}},
          {filterLabel("Leather"), 1, {5}}
        }, {21, {}}},
        {{{filterLabel("Compressed Diamond Hammer"), 1}}, {
          {filterLabel("Diamond Hammer"), 1, {1}},
          {filterLabel("Diamond Hammer"), 1, {2}},
          {filterLabel("Diamond Hammer"), 1, {3}},
          {filterLabel("Diamond Hammer"), 1, {4}},
          {filterLabel("Diamond Hammer"), 1, {5}},
          {filterLabel("Diamond Hammer"), 1, {6}},
          {filterLabel("Diamond Hammer"), 1, {7}},
          {filterLabel("Diamond Hammer"), 1, {8}},
          {filterLabel("Diamond Hammer"), 1, {9}}
        }, {1, {}}},
        {{{filterLabel("Flux"), 16}}, {
          {filterLabel("Redstone"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Block of Black Quartz"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Flux Core"), 64}}, {
          {filterLabel("Flux"), 4, {1, 3, 7, 9}},
          {filterLabel("Obsidian"), 4, {2, 4, 6, 8}},
          {filterLabel("Flux Core"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Lonsdaleite Crystal"), 16}}, {
          {filterLabel("Black Quartz"), 4, {1, 3, 7, 9}},
          {filterLabel("Wither Ash"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Base Crafting Seed"), 16}}, {
          {filterLabel("Prosperity Shard"), 4, {2, 4, 6, 8}},
          {filterLabel("Seeds"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Tier 1 Crafting Seed"), 16}}, {
          {filterLabel(u8"§eInferium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Base Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Tier 2 Crafting Seed"), 16}}, {
          {filterLabel(u8"§aPrudentium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Tier 1 Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Tier 3 Crafting Seed"), 16}}, {
          {filterLabel(u8"§6Intermedium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Tier 2 Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Tier 4 Crafting Seed"), 16}}, {
          {filterLabel(u8"§bSuperium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Tier 3 Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Tier 5 Crafting Seed"), 16}}, {
          {filterLabel(u8"§cSupremium Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Tier 4 Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Void Seed"), 16}}, {
          {filterLabel("Mycelium"), 4, {1, 3, 7, 9}},
          {filterLabel("Lonsdaleite Crystal"), 4, {2, 4, 6, 8}},
          {filterLabel("Tier 5 Crafting Seed"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Ebony Psimetal Ingot"), 16}}, {
          {filterLabel("Ebony Substance"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Psimetal Ingot"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("Ivory Psimetal Ingot"), 16}}, {
          {filterLabel("Ivory Substance"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Psimetal Ingot"), 1, {5}}
        }, {8, {}}},
        {{{filterLabel("1k ME Storage Component"), 3}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Pure Certus Quartz Crystal"), 4, {2, 4, 6, 8}},
          {filterLabel("Logic Processor"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("4k ME Storage Component"), 3}}, {
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("1k ME Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("16k ME Storage Component"), 3}}, {
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("4k ME Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("64k ME Storage Component"), 1}}, {
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("16k ME Storage Component"), 3, {4, 6, 8}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("Calculation Processor"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("Rotten Flesh"), 16}}, {
          {filterLabel("Zombie Essence"), 3, {1, 2, 3}},
        }, {5, {}}},
        {{{filterLabel("Basic Tier Installer"), 16}}, {
          {filterLabel("Block of Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Advanced Control Circuit"), 2, {2, 8}},
          {filterLabel("Steel Ingot"), 2, {4, 6}},
          {filterLabel("Steel Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Advanced Tier Installer"), 16}}, {
          {filterLabel("Reinforced Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Elite Control Circuit"), 2, {2, 8}},
          {filterLabel("Zirconium Ingot"), 2, {4, 6}},
          {filterLabel("Steel Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Elite Tier Installer"), 16}}, {
          {filterLabel("Atomic Alloy"), 4, {1, 3, 7, 9}},
          {filterLabel("Ultimate Control Circuit"), 2, {2, 8}},
          {filterLabel("Blutonium Ingot"), 2, {4, 6}},
          {filterLabel("Steel Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Black Iron Frame"), 4}}, {
          {filterLabel("Black Iron Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Glass"), 4, {2, 4, 6, 8}},
          {filterLabel("Black Iron Slate"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Automation Interface"), 4}}, {
          {filterLabel("Black Iron Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Crystaltine Component"), 2, {4, 6}},
          {filterLabel("Black Iron Slate"), 1, {8}},
          {filterLabel("Black Iron Frame"), 1, {5}},
          {filterLabel("Elite Catalyst"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("Oak Bookshelf"), 16}}, {
          {filterLabel("Ironwood Planks"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Book"), 3, {4, 5, 6}}
        }, {10, {}}},
        {{{filterLabel("Blank Record"), 1}}, {
          {filterLabel("Creeper Essence"), 4, {1, 3, 7, 9}},
          {filterLabel("Skeleton Essence"), 4, {2, 4, 6, 8}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {1, {}}},
        {{{filterName("minecraft:record_13"), 1}}, {
          {filterLabel("Blank Record"), 1, {1}},
          {filterLabel("Dandelion Yellow"), 1, {2}},
          {filterLabel("Skeleton Essence"), 1, {4}},
          {filterLabel("Creeper Essence"), 1, {5}}
        }, {1, {}}}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
