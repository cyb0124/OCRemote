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
      filterLabel("Cobblestone"), filterLabel("Sand"), filterLabel("Dust"), filterLabel("Glowstone Dust"),
      filterLabel("Gravel"), filterLabel("Niter"), filterLabel("Blitz Powder"), filterLabel("Basalz Powder"),
      filterLabel("Flint"), filterLabel(u8"§eInferium Essence"), filterLabel("Iron Ore Piece"), filterLabel("Cocoa Beans"),
      filterLabel("Iron Ore Chunk"), filterLabel("Certus Quartz Crystal"), filterLabel("Platinum Ore Chunk"),
      filterLabel("Platinum Ore Piece"), filterLabel("White Wool"), filterLabel("Blaze Powder"), filterLabel("Bone Meal"),
      filterLabel("Yellorium Ore Piece"), filterLabel("item.contenttweaker.rak_coin.name"), filterLabel("Zinc Ore Piece"),
      filterLabel("Lead Ore Piece"), filterLabel("Nickel Ore Piece"), filterLabel("Archaic Brick"), filterLabel("Silver Ore Piece"),
      filterLabel("Copper Ore Piece"), filterLabel("Tin Ore Piece"), filterLabel("Osmium Ore Piece"), filterLabel("Cake"),
      filterLabel("Nether Wart"), filterLabel("Redstone Seeds"), filterLabel("Rich Slag"), filterLabel("Oak Wood Planks"),
      filterLabel("Grain Bait"), filterLabelName("Nether Brick", "minecraft:nether_brick"), filterLabel("Sky Stone Dust"),
      filterLabel("Nickel Ore Chunk"), filterLabel("Gunpowder"), filterLabel("Certus Quartz Seeds"), filterLabel("Cyanite Ingot"),
      filterName("mysticalagriculture:tier5_inferium_seeds"), filterLabel("Certus Quartz Dust"), filterName("nuclearcraft:flour"),
      filterLabel("Wheat"), filterLabel("Obsidian")
    }));
    factory.addBackup(filterLabel("Seeds"), 32);

    // reactor
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "center"));

    // plasticMixer
    factory.addProcess(std::make_unique<ProcessPlasticMixer>(factory, "plasticMixer", "center"));

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "5cb", Actions::north, Actions::south,
      std::vector<StockEntry>{}, 0, outAll, std::vector<Recipe<int>>{}));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "center", "f80", "a12",
      Actions::east, Actions::north, Actions::west, std::vector<Recipe<std::pair<int, std::optional<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Sugar Canes"), 16}}, {{filterLabel("Nature Essence"), 6, {2, 4, 5, 6, 7, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Cactus"), 16}}, {{filterLabel("Nature Essence"), 7, {1, 2, 3, 5, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Grains of Infinity"), 64}}, {{filterLabel("Grains of Infinity Essence"), 5, {2, 4, 5, 6, 8}}}, {2, std::nullopt}},
        {{{filterLabel("Vibrant Alloy Grinding Ball"), 16}}, {{filterLabel("Vibrant Alloy Ingot"), 5, {2, 4, 5, 6, 8}}}, {2, std::nullopt}},
        {{{filterLabel("Bone"), 64}}, {{filterLabel("Skeleton Essence"), 5, {2, 4, 5, 6, 8}}}, {8, std::nullopt}},
        {{{filterLabel("Slimeball"), 64}}, {{filterLabel("Slime Essence"), 5, {2, 4, 5, 6, 8}}}, {8, std::nullopt}},
        {{{filterLabel("Netherrack"), 64}}, {{filterLabel("Nether Essence"), 5, {2, 4, 5, 6, 8}}}, {2, std::nullopt}},
        {{{filterLabel("Blizz Rod"), 64}}, {{filterLabel("Blizz Essence"), 5, {2, 4, 5, 6, 8}}}, {12, std::nullopt}},
        {{{filterLabel("Blaze Rod"), 64}}, {{filterLabel("Blaze Essence"), 5, {2, 4, 5, 6, 8}}}, {12, std::nullopt}},
        {{{filterLabel("Sky Stone"), 64}}, {{filterLabel("Sky Stone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {2, std::nullopt}},
        {{{filterLabel("Soul Sand"), 64}}, {{filterLabel("Nether Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {3, std::nullopt}},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Yellorium Ingot"), 64}}, {{filterLabel("Yellorium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Enderium Ingot"), 64}}, {{filterLabel("Enderium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Signalum Ingot"), 64}}, {{filterLabel("Signalum Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Compressed Iron Ingot"), 64}}, {{filterLabel("Compressed Iron Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Lumium Ingot"), 64}}, {{filterLabel("Lumium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Gold Ingot"), 64}}, {{filterLabel("Gold Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Dawnstone Ingot"), 64}}, {{filterLabel("Dawnstone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Fluix Crystal"), 64}}, {{filterLabel("Fluix Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Lapis Lazuli"), 64}}, {{filterLabel("Lapis Lazuli Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {5, std::nullopt}},
        {{{filterLabel("Glowstone Dust"), 64}}, {{filterLabel("Glowstone Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Certus Quartz Crystal"), 64}}, {{filterLabel("Certus Quartz Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {8, std::nullopt}},
        {{{filterLabel("Redstone"), 64}}, {{filterLabel("Redstone Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Diamond Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Compressed Netherrack"), 16}}, {{filterLabel("Netherrack"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Emerald"), 16}}, {{filterLabel("Emerald Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Coal"), 64}}, {{filterLabel("Coal Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}}, {5, std::nullopt}},
        {{{filterLabel("tile.compressed_dust.name"), 16}}, {{filterLabel("Dust"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Bone Block"), 16}}, {{filterLabel("Bone Meal"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Block of Compressed Iron"), 16}}, {{filterLabel("Compressed Iron Ingot"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}}, {7, std::nullopt}},
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 5, 9}}}, {4, std::nullopt}},
        {{{filterLabel("Oak Wood"), 64}}, {{filterLabel("Wood Essence"), 3, {1, 2, 3}}}, {4, std::nullopt}},
        {{{filterLabel("Sandstone"), 16}}, {{filterLabel("Sand"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("String"), 64}}, {{filterLabel("Cotton"), 3, {1, 2, 4}}}, {20, std::nullopt}},
        {{{filterLabel("White Wool"), 64}}, {{filterLabel("String"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Bronze Gear"), 16}}, {{filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Silver Gear"), 16}}, {{filterLabel("Silver Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Lumium Gear"), 16}}, {{filterLabel("Lumium Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Electrum Gear"), 16}}, {{filterLabel("Electrum Ingot"), 4, {2, 4, 6, 8}}}, {16, std::nullopt}},
        {{{filterLabel("Weighted Pressure Plate (Heavy)"), 16}}, {{filterLabel("Iron Ingot"), 2, {1, 2}}}, {16, std::nullopt}},
        {{{filterLabel("Weighted Pressure Plate (Light)"), 16}}, {{filterLabel("Gold Ingot"), 2, {1, 2}}}, {16, std::nullopt}},
        {{{filterLabel("Paper"), 64}}, {{filterLabel("Sawdust"), 3, {4, 5, 6}}}, {10, std::nullopt}},
        {{{filterLabel("Iron Bars"), 16}}, {{filterLabel("Iron Ingot"), 6, {1, 2, 3, 4, 5, 6}}}, {4, std::nullopt}},
        {{{filterLabel("Dandelion Yellow"), 16}}, {{filterLabel("Dye Essence"), 3, {1, 3, 5}}}, {10, std::nullopt}},
        {{{filterLabel("Rose Red"), 16}}, {{filterLabel("Dye Essence"), 3, {4, 5, 6}}}, {10, std::nullopt}},
        {{{filterLabel("Sulfur"), 16}}, {{filterLabel("Sulfur Essence"), 3, {1, 2, 3}}}, {8, std::nullopt}},
        {{{filterLabel("Blank Skull"), 16}}, {
          {filterLabel("Bone Block"), 1, {5}},
          {filterLabel("Soul Dust"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Pure Certus Quartz Crystal"), 64}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Certus Quartz Dust"), 1, {2}}
        }, {32, std::nullopt}},
        {{{filterLabel("Pure Nether Quartz Crystal"), 64}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Crushed Quartz"), 1, {2}}
        }, {32, std::nullopt}},
        {{{filterLabel("Pure Fluix Crystal"), 64}}, {
          {filterLabel("Sand"), 1, {1}},
          {filterLabel("Fluix Dust"), 1, {2}}
        }, {32, std::nullopt}},
        {{{filterLabel("Quartz Glass"), 16}}, {
          {filterLabel("Crushed Quartz"), 5, {1, 3, 5, 7, 9}},
          {filterLabel("Glass"), 4, {2, 4, 6, 8}}
        }, {12, std::nullopt}},
        {{{filterLabel("Rich Phyto-Gro"), 16}}, {
          {filterLabel("Pulverized Charcoal"), 1, {1}},
          {filterLabel("Niter"), 1, {2}},
          {filterLabel("Rich Slag"), 1, {3}}
        }, {4, std::nullopt}},
        {{{filterLabelName("Clay", "minecraft:clay_ball"), 16}}, {
          {filterLabel("Water Essence"), 2, {1, 5}},
          {filterLabel("Dirt Essence"), 2, {2, 4}}
        }, {2, std::nullopt}},
        {{{filterLabel("Cryotheum Dust"), 16}}, {
          {filterLabel("Blizz Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {4}},
          {filterLabel("Snowball"), 1, {5}}
        }, {32, std::nullopt}},
        {{{filterLabel("Pyrotheum Dust"), 16}}, {
          {filterLabel("Blaze Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {4}},
          {filterLabel("Sulfur"), 1, {5}}
        }, {32, std::nullopt}},
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
        }, {32, std::nullopt}},
        {{{filterLabel("Augment: Auxiliary Reception Coil"), 4}}, {
          {filterLabel("Redstone Reception Coil"), 1, {5}},
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Augment: Auxiliary Sieve"), 4}}, {
          {filterLabel("Redstone Servo"), 1, {5}},
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Hardened Upgrade Kit"), 4}}, {
          {filterLabel("Bronze Gear"), 1, {5}},
          {filterLabel("Redstone"), 2, {7, 9}},
          {filterLabel("Invar Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Reinforced Upgrade Kit"), 4}}, {
          {filterLabel("Silver Gear"), 1, {5}},
          {filterLabel("Hardened Glass"), 2, {7, 9}},
          {filterLabel("Electrum Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Signalum Upgrade Kit"), 4}}, {
          {filterLabel("Electrum Gear"), 1, {5}},
          {filterLabel("Cryotheum Dust"), 2, {7, 9}},
          {filterLabel("Signalum Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Resonant Upgrade Kit"), 4}}, {
          {filterLabel("Lumium Gear"), 1, {5}},
          {filterLabel("Pyrotheum Dust"), 2, {7, 9}},
          {filterLabel("Enderium Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
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
        {{{filterLabel("1k ME Storage Component"), 3}}, {
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Pure Certus Quartz Crystal"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("4k ME Storage Component"), 3}}, {
          {filterLabel("Calculation Processor"), 1, {2}},
          {filterLabel("Redstone"), 4, {1, 3, 7, 9}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("1k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("16k ME Storage Component"), 3}}, {
          {filterLabel("Calculation Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("4k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("64k ME Storage Component"), 3}}, {
          {filterLabel("Calculation Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Quartz Glass"), 1, {5}},
          {filterLabel("16k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("256k ME Storage Component"), 3}}, {
          {filterLabel("Engineering Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("64k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("1024k ME Storage Component"), 3}}, {
          {filterLabel("Engineering Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("256k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("4096k ME Storage Component"), 3}}, {
          {filterLabel("Engineering Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("1024k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("16384k ME Storage Component"), 3}}, {
          {filterLabel("Engineering Processor"), 1, {2}},
          {filterLabel("Glowstone Dust"), 4, {1, 3, 7, 9}},
          {filterLabel("Logic Processor"), 1, {5}},
          {filterLabel("4096k ME Storage Component"), 3, {4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Raw Tofeeg"), 16}}, {
          {filterLabel("Firm Tofu"), 1, {2}},
          {filterLabel("Dandelion Yellow"), 1, {4}}
        }, {32, NonConsumableInfo{27, 1}}},
        {{{filterLabel("Cake"), 64}}, {
          {filterLabel("Soy Milk"), 3, {1, 2, 3}},
          {filterLabel("Sugar"), 2, {4, 6}},
          {filterLabel("Raw Tofeeg"), 1, {5}},
          {filterLabel("Wheat"), 3, {7, 8, 9}}
        }, {21, std::nullopt}},
        {{{filterLabel("Simple Machine Chassis"), 4}}, {
          {filterLabel("Grains of Infinity"), 1, {5}},
          {filterLabel("Iron Bars"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Ingot"), 4, {2, 4, 6, 8}}
        }, {16, std::nullopt}},
        {{{filterLabel("Industrial Dye Blend"), 16}}, {
          {filterLabel("Organic Black Dye"), 1, {5}},
          {filterLabel("Crushed Lapis"), 2, {1, 9}},
          {filterLabel("Organic Green Dye"), 2, {3, 7}},
          {filterLabel("Crushed Quartz"), 4, {2, 4, 6, 8}}
        }, {10, std::nullopt}},
        {{{filterLabel("Soulstone"), 16}}, {
          {filterLabel("Stone"), 5, {1, 3, 5, 7, 9}},
          {filterLabel("Soul Sand"), 4, {2, 4, 6, 8}}
        }, {8, std::nullopt}},
        {{{filterLabel("Skeleton Skull"), 16}}, {
          {filterLabel("Blank Skull"), 1, {5}},
          {filterLabel("Skeleton Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, std::nullopt}},
        {{{filterLabel("Conduit Binder Composite"), 16}}, {
          {filterLabel("Gravel"), 5, {1, 3, 5, 7, 9}},
          {filterLabelName("Clay", "minecraft:clay_ball"), 2, {2, 8}},
          {filterLabel("Sand"), 2, {4, 6}}
        }, {8, std::nullopt}},
        {{{filterLabel("Yellorium Ore Chunk"), 16}}, {{filterLabel("Yellorium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Platinum Ore Chunk"), 16}}, {{filterLabel("Platinum Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Mithril Ore Chunk"), 16}}, {{filterLabel("Mithril Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Copper Ore Chunk"), 16}}, {{filterLabel("Copper Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Osmium Ore Chunk"), 16}}, {{filterLabel("Osmium Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Silver Ore Chunk"), 16}}, {{filterLabel("Silver Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Nickel Ore Chunk"), 16}}, {{filterLabel("Nickel Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Lead Ore Chunk"), 16}}, {{filterLabel("Lead Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Gold Ore Chunk"), 16}}, {{filterLabel("Gold Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Iron Ore Chunk"), 16}}, {{filterLabel("Iron Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Zinc Ore Chunk"), 16}}, {{filterLabel("Zinc Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}},
        {{{filterLabel("Tin Ore Chunk"), 16}}, {{filterLabel("Tin Ore Piece"), 4, {1, 2, 4, 5}}}, {16, std::nullopt}}
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
      std::vector<StockEntry>{}, 128, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Sand"), 640}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Niter"), 16}}, {{filterLabel("Sandstone"), 1}}, INT_MAX},
        {{{filterLabel("Silicon"), 16}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Charcoal"), 16}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Obsidian"), 16}}, {{filterLabel("Obsidian"), 1}}, INT_MAX},
        {{{filterLabel("Pulverized Lead"), 16}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Dust"), 16}}, {{filterLabel("Graphite Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Crushed Quartz"), 16}}, {{filterLabel("Nether Quartz"), 1}}, INT_MAX},
        {{{filterLabel("Crushed Lapis"), 16}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
        {{{filterLabel("Certus Quartz Dust"), 16}}, {{filterLabel("Certus Quartz Crystal"), 1}}, INT_MAX},
        {{{filterLabel("Fluix Dust"), 16}}, {{filterLabel("Fluix Crystal"), 1}}, INT_MAX},
        {{{filterLabel("Stick"), 64}}, {{filterLabel("Oak Wood Planks"), 1}}, INT_MAX}
      }));

    // sagMill
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sagMill", "center", "c28", Actions::north, Actions::east,
      std::vector<StockEntry>{
        {filterLabel("Vibrant Alloy Grinding Ball"), 16},
        {filterLabel("Seeds"), 64},
        {filterLabel("Sugar Canes"), 64}
      }, 64, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Crushed Netherrack"), 64}}, {{filterLabel("Compressed Netherrack"), 1}}, INT_MAX},
        {{{filterLabel("Bone Meal"), 16}}, {{filterLabel("Bone"), 1}}, INT_MAX},
        {{{filterLabel("Seeds"), 64}}, {{filterLabel("Wheat"), 1}}, INT_MAX},
        {{{filterLabel("Blaze Powder"), 16}, {filterLabel("Sulfur"), 16}}, {{filterLabel("Blaze Rod"), 1}}, INT_MAX},
        {{}, {{filterLabel("Charged Certus Quartz Crystal"), 1}}, INT_MAX}
      }));

    // pneumaticSharedBuffer
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pneumaticSharedBuffer", "center", "bc3", Actions::up, Actions::east,
      std::vector<StockEntry>{
        {filterLabel("Pyrotheum Dust"), 32},
        {filterLabel("Coal"), 16},
        {filterLabel("Rose Red"), 16},
        {filterLabel("Cactus Green"), 16},
        {filterLabel("Lapis Lazuli"), 16}
      }, INT_MAX, nullptr, std::vector<Recipe<int>>{}));

    // pneumaticAssembly
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pneumaticAssembly", "center", "c95", Actions::south, Actions::west,
      std::vector<StockEntry>{}, 8, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Unassembled PCB"), 16}}, {{filterName("pneumaticcraft:empty_pcb"), 1}}, INT_MAX},
        {{{filterLabel("Advanced Pressure Tube"), 16}}, {{filterLabel("Block of Compressed Iron"), 1}}, INT_MAX}
      }));

    // pressureChamber
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pressureChamber", "center", "bc3", Actions::south, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, std::vector<Recipe<int>>{
        {{{filterName("pneumaticcraft:turbine_blade"), 16}}, {
          {filterLabel("Redstone"), 2},
          {filterLabel("Gold Ingot"), 1}
        }, INT_MAX},
        {{{filterName("pneumaticcraft:empty_pcb"), 16}}, {
          {filterLabel("Green Plastic"), 1},
          {filterLabel("Compressed Iron Ingot"), 1}
        }, INT_MAX},
        {{{filterName("pneumaticcraft:capacitor"), 16}}, {
          {filterLabel("Cyan Plastic"), 1},
          {filterLabel("Compressed Iron Ingot"), 1},
          {filterLabel("Redstone"), 1}
        }, INT_MAX},
        {{{filterName("pneumaticcraft:transistor"), 16}}, {
          {filterLabel("Black Plastic"), 1},
          {filterLabel("Compressed Iron Ingot"), 1},
          {filterLabel("Redstone"), 1}
        }, INT_MAX}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "furnace", "center", "a12", Actions::west, Actions::north,
      std::vector<StockEntry>{}, 96, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Soul Dust"), 16}}, {{filterLabel("Soulstone"), 1}}, INT_MAX},
        {{{filterLabel("Glass"), 64}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Stone"), 64}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Brick"), 16}}, {{filterLabel("Clay"), 1}}, INT_MAX},
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Oak Wood"), 1}}, INT_MAX},
        {{{filterLabel("Graphite Ingot"), 64}}, {{filterLabel("Charcoal"), 1}}, INT_MAX},
        {{{filterLabel("Cactus Green"), 16}}, {{filterLabel("Cactus"), 1}}, INT_MAX},
        {{{filterLabel("Conduit Binder"), 64}}, {{filterLabel("Conduit Binder Composite"), 1}}, INT_MAX}
      }));

    // aeSharedBuffer
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "aeSharedBuffer", "center", "c28", Actions::south, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, std::vector<Recipe<int>>{
        {{}, {{filterLabel("Certus Quartz Seed"), 1}}, INT_MAX},
        {{}, {{filterLabel("Nether Quartz Seed"), 1}}, INT_MAX},
        {{}, {{filterLabel("Fluix Seed"), 1}}, INT_MAX},
        {{{filterLabel("Printed Calculation Circuit"), 16}}, {{filterLabel("Pure Certus Quartz Crystal"), 1}}, 64},
        {{{filterLabel("Printed Engineering Circuit"), 16}}, {{filterLabel("Diamond"), 1}}, 64},
        {{{filterLabel("Printed Logic Circuit"), 16}}, {{filterLabel("Gold Ingot"), 1}}, 64},
        {{{filterLabel("Printed Silicon"), 16}}, {{filterLabel("Silicon"), 1}}, 64},
        {{{filterLabel("Calculation Processor"), 16}}, {
          {filterLabel("Printed Silicon"), 1},
          {filterLabel("Redstone"), 1},
          {filterLabel("Printed Calculation Circuit"), 1}
        }, 192},
        {{{filterLabel("Engineering Processor"), 16}}, {
          {filterLabel("Printed Silicon"), 1},
          {filterLabel("Redstone"), 1},
          {filterLabel("Printed Engineering Circuit"), 1}
        }, 192},
        {{{filterLabel("Logic Processor"), 16}}, {
          {filterLabel("Printed Silicon"), 1},
          {filterLabel("Redstone"), 1},
          {filterLabel("Printed Logic Circuit"), 1}
        }, 192},
        {{{filterLabel("Oak Wood Planks"), 64}}, {{filterLabel("Oak Wood"), 1}}, 64},
        {{{filterLabel("Sugar"), 16}}, {{filterLabel("Sugar Canes"), 1}}, 64},
        {{{filterLabel("Silken Tofu"), 16}}, {{filterLabel("Soybean"), 1}}, 64},
        {{
          {filterLabel("Soy Milk"), 16},
          {filterLabel("Firm Tofu"), 16}
        }, {{filterLabel("Silken Tofu"), 1}}, 64},
        {{}, {{filterName("lootbags:itemlootbag"), 1}}, INT_MAX}
      }));

    // pulverizer
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pulverizer", "center", "c28", Actions::west, Actions::east,
      std::vector<StockEntry>{{filterLabel("Nether Wart"), 16}, {filterLabel("Skeleton Skull"), 16}}, 96, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Dust"), 640}}, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{{filterLabel("Gravel"), 640}}, {{filterLabel("Cobblestone"), 1}}, INT_MAX},
        {{{filterLabel("Sawdust"), 16}}, {{filterLabel("Oak Wood"), 1}}, INT_MAX},
        {{{filterLabel("Blizz Powder"), 16}}, {{filterLabel("Blizz Rod"), 1}}, INT_MAX}
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "a12", Actions::south, Actions::north,
      std::vector<StockEntry>{
        {filterLabel("Fluxed Phyto-Gro"), 32},
        {filterLabel("Yellorium Ingot"), 16},
        {filterLabel("Cobblestone"), 16},
        {filterLabel("Cryotheum Dust"), 16}
      }, 32, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel(u8"§eInferium Essence"), 80'000}}, {{filterName("mysticalagriculture:tier5_inferium_seeds"), 1}}, INT_MAX},
        {{{filterLabel("Grains of Infinity Essence"), 16}}, {{filterLabel("Grains of Infinity Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Compressed Iron Essence"), 16}}, {{filterLabel("Compressed Iron Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Certus Quartz Essence"), 16}}, {{filterLabel("Certus Quartz Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Lapis Lazuli Essence"), 16}}, {{filterLabel("Lapis Lazuli Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Sky Stone Essence"), 16}}, {{filterLabel("Sky Stone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Glowstone Essence"), 16}}, {{filterLabel("Glowstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Yellorium Essence"), 16}}, {{filterLabel("Yellorium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Dawnstone Essence"), 16}}, {{filterLabel("Dawnstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 16}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Enderium Essence"), 16}}, {{filterLabel("Enderium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Signalum Essence"), 16}}, {{filterLabel("Signalum Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Skeleton Essence"), 16}}, {{filterLabel("Skeleton Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Diamond Essence"), 16}}, {{filterLabel("Diamond Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Emerald Essence"), 16}}, {{filterLabel("Emerald Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Lumium Essence"), 16}}, {{filterLabel("Lumium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Slime Essence"), 16}}, {{filterLabel("Slime Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Essence"), 16}}, {{filterLabel("Cobalt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nether Essence"), 16}}, {{filterLabel("Nether Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nature Essence"), 16}}, {{filterLabel("Nature Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Sulfur Essence"), 16}}, {{filterLabel("Sulfur Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Fluix Essence"), 16}}, {{filterLabel("Fluix Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Water Essence"), 16}}, {{filterLabel("Water Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Blizz Essence"), 16}}, {{filterLabel("Blizz Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Blaze Essence"), 16}}, {{filterLabel("Blaze Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Gold Essence"), 16}}, {{filterLabel("Gold Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Wood Essence"), 16}}, {{filterLabel("Wood Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Dirt Essence"), 16}}, {{filterLabel("Dirt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Coal Essence"), 16}}, {{filterLabel("Coal Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Dye Essence"), 16}}, {{filterLabel("Dye Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Ender Pearl"), 64}}, {{filterLabel("Ender Lilly"), 1}}, INT_MAX},
        {{{filterLabel("Soybean"), 16}}, {{filterLabel("Soybean Seed"), 1}}, INT_MAX},
        {{{filterLabel("Cotton"), 16}}, {{filterLabel("Cotton Seeds"), 1}}, INT_MAX},
        {{
          {filterLabel("Seeds"), 64},
          {filterLabel("Wheat"), 64}
        }, {{filterLabel("Seeds"), 1, {}, true}}, INT_MAX}
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
        {{{filterLabel("Yellorium Ingot"), 64}}, {{filterLabel("Yellorium Ore Chunk"), 1}}, INT_MAX},
        {{{filterLabel("Mana Infused Ingot"), 64}}, {{filterLabel("Mithril Ore Chunk"), 1}}, INT_MAX}
      }));

    // sieve
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sieve", "center", "6ef", Actions::east, Actions::west,
      std::vector<StockEntry>{}, 64, nullptr, std::vector<Recipe<int>>{
        {{
          {filterLabel("Tin Ore Piece"), 16},
          {filterLabel("Iron Ore Piece"), 16},
          {filterLabel("Gold Ore Piece"), 16},
          {filterLabel("Lead Ore Piece"), 16},
          {filterLabel("Zinc Ore Piece"), 16},
          {filterLabel("Copper Ore Piece"), 16},
          {filterLabel("Osmium Ore Piece"), 16},
          {filterLabel("Silver Ore Piece"), 16},
          {filterLabel("Nickel Ore Piece"), 16},
          {filterLabel("Platinum Ore Piece"), 16},
          {filterLabel("Yellorium Ore Piece"), 16}
        }, {{filterLabel("Sand"), 1}}, INT_MAX},
        {{
          {filterLabel("Prosperity Shard"), 64},
          {filterLabel("Mithril Ore Piece"), 16}
        }, {{filterLabel("Crushed Netherrack"), 1}}, INT_MAX},
        {{{filterLabel("Coal"), 64}}, {{filterLabel("Gravel"), 1}}, INT_MAX},
        {{
          {filterLabel("Nether Quartz"), 64},
          {filterLabel("Nether Wart"), 64}
        }, {{filterLabel("Soul Sand"), 1}}, INT_MAX},
        {{
          {filterLabel("Redstone"), 64},
          {filterLabel("Glowstone Dust"), 64},
          {filterLabel("Niter"), 16},
          {filterLabel("Blaze Powder"), 16},
          {filterLabel("Blizz Powder"), 16}
        }, {{filterLabel("tile.compressed_dust.name"), 1}}, INT_MAX}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "center", "f80", Actions::up, Actions::east,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 64}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "center", "6ef", Actions::down, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Steel Ingot"), 64}}, {{filterLabel("Iron Ingot"), 1, {0}}, {filterLabel("Graphite Ingot"), 1, {1}}}, 64},
        {{{filterLabel("Bronze Ingot"), 64}}, {{filterLabel("Copper Ingot"), 3, {0}}, {filterLabel("Tin Ingot"), 1, {1}}}, 16},
        {{{filterLabel("Invar Ingot"), 64}}, {{filterLabel("Iron Ingot"), 2, {0}}, {filterLabel("Nickel Ingot"), 1, {1}}}, 32},
        {{{filterLabel("Electrum Ingot"), 64}}, {{filterLabel("Gold Ingot"), 1, {0}}, {filterLabel("Silver Ingot"), 1, {1}}}, 64},
        {{{filterLabel("Electrical Steel Ingot"), 64}}, {{filterLabel("Steel Ingot"), 1, {0}}, {filterLabel("Silicon"), 1, {1}}}, 64},
        {{{filterLabel("Dark Steel Ingot"), 64}}, {{filterLabel("Steel Ingot"), 1, {0}}, {filterLabel("Obsidian"), 1, {1}}}, 64},
        {{{filterLabel("Vibrant Alloy Ingot"), 64}}, {{filterLabel("Energetic Alloy Ingot"), 1, {0}}, {filterLabel("Ender Pearl"), 1, {1}}}, 64},
        {{{filterLabel("Pulsating Iron Ingot"), 64}}, {{filterLabel("Iron Ingot"), 1, {0}}, {filterLabel("Ender Pearl"), 1, {1}}}, 64}
      }));

    // alloySmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloySmelter", "center", "c28", Actions::up, Actions::east,
      std::vector<size_t>{0, 1, 2}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Organic Green Dye"), 16}}, {
          {filterLabel("Cactus Green"), 2, {0}},
          {filterLabel("Slimeball"), 1, {1}},
          {filterLabel("Pulverized Charcoal"), 2, {2}}
        }, 32},
        {{{filterLabel("Organic Black Dye"), 16}}, {
          {filterLabel("Pulverized Charcoal"), 6, {0}},
          {filterLabel("Slimeball"), 1, {1}}
        }, 10},
        {{{filterLabel("Energetic Alloy Ingot"), 64}}, {
          {filterLabel("Redstone"), 1, {0}},
          {filterLabel("Gold Ingot"), 1, {1}},
          {filterLabel("Glowstone Dust"), 1, {2}}
        }, 10}
      }));

    // inductionSmelter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "inductionSmelter", "center", "3c3", Actions::north, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Hardened Glass"), 16}}, {{filterLabel("Pulverized Lead"), 1, {0}}, {filterLabel("Pulverized Obsidian"), 4, {1}}}, 16},
        {{{filterLabel("Sulfur"), 16}}, {{filterLabel("Soul Sand"), 1, {0}}, {filterLabel("Netherrack"), 2, {1}}}, 32},
        {{{filterLabel("Industrial Machine Chassis"), 16}}, {{filterLabel("Simple Machine Chassis"), 1, {0}}, {filterLabel("Industrial Dye Blend"), 1, {1}}}, 64}
      }));

    factory.start();
    io.io.run();
    return EXIT_SUCCESS;
  } catch (std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
