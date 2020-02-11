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
      filterLabel("Cobblestone"), filterLabel("Gravel"), filterLabel("Gunpowder"), filterName("harvestcraft:flouritem"),
      filterLabel("Blaze Powder"), filterLabel("Redstone"), filterLabel("Glowstone Dust"), filterLabel("Nether Quartz"),
      filterLabel("Lapis Lazuli"), filterLabel("Certus Quartz Crystal"), filterLabel("Diamond"), filterLabel("Emerald"),
      filterLabel("Birch Wood Planks"), filterLabel("Stick"), filterLabel("Sawdust"), filterLabel("Ground Cinnamon"),
      filterLabel("Sand"), filterLabel("Glass"), filterLabel("Stone"), filterLabel("Netherrack"),
      filterLabel("Potato"), filterLabel("Treated Wood Planks"), filterLabel("Compass"), filterLabel("Compressed Ender Gravel"),
      filterLabel("Birch Wood"), filterLabel("Birch Sapling"), filterLabel("Charcoal"), filterLabel("Coal"),
      filterLabel("Crushed Stone"), filterLabel("Dirty Emerald"), filterLabel("Dirty Diamond"), filterLabel("Dirty Ruby"),
      filterLabel("Dirty Sapphire"), filterLabel("Dirty Peridot"), filterLabel("Dirty Apatite"), filterLabel("Dirty Amber"),
      filterLabel("Dirty Aquamarine"), filterLabel("Dirty Lapis Lazuli"), filterLabel("Dirty Black Quartz"), filterLabel("Dirty Certus Quartz"),
      filterLabel("Dirty Charged Certus Quartz"), filterLabel("Dirty Malachite"), filterLabel("Raw Toficken"), filterLabel("Cyanite Ingot"),
      filterLabel("Wheat"), filterLabel("Seeds"), filterLabel("Tertius Alchemical Dust"), filterLabel("Crushed Endstone"),
      filterLabel("Gold Ingot"), filterLabel("Alchemical Gold Ingot"), filterLabel("Osmium Essence"), filterLabel("Magnesium Ore Piece"),
      filterLabel("Osmium Ingot"), filterLabel("Osmium Seeds"), filterLabel("Iron Seeds"), filterLabel("Aluminum Ingot"),
      filterLabel("Cobalt Ingot"), filterLabel("Compressed Nether Gravel"), filterLabel("Gold Ore Piece"), filterLabel("Ardite Ore Piece"),
      filterLabel("Ardite Ingot"), filterLabel("Iron Essence"), filterLabel("Iron Ingot"), filterLabel("Thorium Ore Piece"),
      filterLabel("Aluminum Seeds"), filterLabel("Aluminum Essence"), filterLabel("Nickel Ingot"), filterLabel("Nickel Essence"),
      filterLabel("Silver Ingot"), filterLabel("Silver Essence"), filterLabel("Copper Ingot"), filterLabel("Lead Essence"),
      filterLabel("Silver Seeds"), filterLabel("Copper Seeds"), filterLabel("Copper Essence"), filterLabel("Lead Ingot"),
      filterLabel("Tin Ingot"), filterLabel("Lead Seeds"), filterLabel("Tin Essence"), filterLabel("Tin Seeds"),
      filterLabel("Aquamarine"), filterLabel("Plant Matter"), filterLabel("Graphite Ingot"), filterLabel("Rice Seeds"),
      filterLabel("String"), filterLabel(u8"§eInferium Essence"), filterLabel("Rotten Flesh"), filterLabel("Bone"),
      filterLabel("Arrow"), filterLabel("Spider Eye"), filterLabel("Zombie Brain"), filterLabel("Ender Pearl"),
      filterLabel("Bronze Ingot"), filterLabel("Steel Ingot"), filterLabel("Invar Ingot"), filterLabel("Silicon Ingot"),
      filterLabel("Electrical Steel Ingot"), filterLabel("Electrum Ingot"), filterLabel("Tiny Dry Rubber"), filterLabel("Dry Rubber"),
      filterLabel("Sugar Canes"), filterLabel("Soybean"), filterLabel("Sugar"), filterLabel("Pulverized Obsidian"),
      filterLabel("Silken Tofu"), filterLabel("Grain Bait"), filterLabel("Firm Tofu"), filterLabel("Soy Milk"),
      filterLabel("Plastic"), filterLabel("Dirt"), filterLabel("Stardust"), filterLabel("Deluxe Chicken Curry"),
      filterLabel("Uranium Ingot"), filterLabel("Corned Beef Breakfast"), filterLabel("End Stone"), filterLabel("Compressed End Stone"),
      filterLabel("Iron Plate"), filterLabel("Block of Iron"), filterLabel("Iron Large Plate"), filterLabel("Aluminum Plate"),
      filterLabel("Rice"), filterLabel("Paper"), filterLabel("Rice Dough"), filterLabel("Basic Machine Casing"),
      filterLabel("Bronze Plate"), filterLabel("Tin Plate"), filterLabel("Steel Plate"), filterLabel("Mixed Metal Ingot"),
      filterLabel("Advanced Alloy"), filterLabel("Pulverized Coal"), filterLabel("Raw Carbon Fibre"), filterLabel("Raw Carbon Mesh"),
      filterLabel("Carbon Plate"), filterLabel("Advanced Machine Casing"), filterLabel("Water Seeds"), filterLabel("Water Essence"),
      filterLabel("Uranium Plate"), filterName("minecraft:clay"), filterName("minecraft:clay_ball"), filterLabel("Clay Dust"),
      filterLabel("Reinforced Stone"), filterLabel("Grout"), filterLabel("Machine Case"), filterLabel("Rice Slimeball"),
      filterLabel("Green Slime Block"), filterLabel("Slimeball"), filterLabel("Blaze Powder Block"), filterLabel("Obsidian"),
      filterLabel("Enori Crystal"), filterLabel("Copper Gear"), filterLabel("Pink Slime"), filterLabel("Restonia Crystal"),
      filterLabel("Gold Cable"), filterLabel("Aluminium Wire"), filterLabel("Basic Coil"), filterLabel("Advanced Coil"),
      filterLabel("Device Frame"), filterLabel("Copper Cable"), filterLabel("Iron Sheetmetal"), filterLabel("Gold Gear"),
      filterLabel("Glass Pane"), filterLabel("Steel Sheetmetal"), filterLabel("Iron Mechanical Component"), filterLabel("Coil"),
      filterLabel("Sturdy Casing"), filterLabel("Bronze Gear"), filterLabel("Steel Rod"), filterLabel("Compressed Redstone"),
      filterLabel("Compressed Diamond"), filterLabel("Enriched Alloy"), filterLabel("Basic Control Circuit"), filterName("rftools:machine_frame"),
      filterLabel("Tin Item Casing"), filterLabel("Iron Bars"), filterLabel("Electric Motor"), filterLabel("Heat Vent"),
      filterLabel("Reinforced Alloy"), filterLabel("Steel Scaffolding"), filterLabel("Heavy Engineering Block"), filterLabel("Tin Electron Tube"),
      filterLabel("Hardened Casing"), filterLabel("Iron Casing"), filterLabelName("Machine Frame", "thermalexpansion:frame"), filterLabel("Tomato"),
      filterLabel("Rich Slag"), filterLabel("Sandstone"), filterLabel("Niter"), filterLabel("Pulverized Charcoal"),
      filterLabel("Rich Phyto-Gro"), filterLabel("Fluxed Phyto-Gro"), filterLabel("Dye Seeds"), filterLabel("Dye Essence"),
      filterLabel("Dandelion Yellow"), filterLabel("Raw Tofeeg"), filterName("minecraft:red_mushroom"), filterLabel("Compressed Cobblestone"),
      filterLabel("Piston"), filterLabel("Soul Sand"), filterLabel("Dirt Seeds"), filterLabel("Dirt Essence"),
      filterLabel("Nature Seeds"), filterLabel("Nature Essence"), filterName("minecraft:brown_mushroom"), filterLabel("Compressed Netherrack"),
      filterName("exnihilocreatio:block_netherrack_crushed"), filterLabel("Lithium Ore Piece"), filterLabel("Boron Ore Piece"), filterLabel("Cobalt Ore Piece"),
      filterLabel("Boron Ore"), filterLabel("Magnesium Ore"), filterLabel("Lithium Ore"), filterLabel("Cobalt Ore"),
      filterLabel("Gold Ore"), filterLabel("Ardite Ore"), filterLabel("Thorium Ore"), filterLabel("Boron Ingot"),
      filterLabel("Thorium Ingot"), filterLabel("Magnesium Ingot"), filterLabel("Lithium Ingot"), filterLabel("Creeper Head"),
      filterLabel("Ferroboron Alloy"), filterLabel("Tough Alloy"), filterLabel("Lead Item Casing"), filterLabel("Lead Plate"),
      filterLabel("Lead Sheetmetal"), filterLabel("Graphite Block"), filterLabel("Basic Plating"), filterLabel("Aluminium Rod"),
      filterLabel("Aluminium Sheetmetal"), filterLabel("Copper Item Casing"), filterLabel("Copper Solenoid"), filterLabel("Copper Plate"),
      filterLabel("Leather"), filterLabel("Book"), filterLabel("Ketchup"), filterLabelName("Flour", "appliedenergistics2:material"),
      filterLabel("Bread"), filterLabel("Brick"), filterLabel("Button"), filterLabel("Chest"),
      filterLabel("Hopper"), filterLabel("Redstone Torch"), filterLabel("Gold Nugget"), filterLabel("Cactus Green"),
      filterLabel("Cactus"), filterLabel("Bucket"), filterLabel("Dropper"), filterLabel("Dispenser"),
      filterLabel("Glass Bottle"), filterLabel("Raw Circuit Board"), filterLabel("Printed Circuit Board (PCB)"), filterLabel("Transistor"),
      filterLabel("Analyzer"), filterLabel("Microchip (Tier 2)"), filterLabel("Inventory Controller Upgrade"), filterLabel("Tank Controller Upgrade"),
      filterLabel("Transposer"), filterLabel("Energy Laser Relay"), filterLabel("Item Interface"), filterLabel("Block of Redstone"),
      filterLabel("Fresh Water"), filterLabel("Salt"), filterLabel("Butter"), filterLabel("Toast"),
      filterLabel("Cheese"), filterLabel("Bellpepper"), filterLabel("Onion"), filterLabel("Mustard Seeds"),
      filterLabel("Spice Leaf"), filterLabel("Ginger"), filterLabel("Microchip (Tier 1)"), filterLabel("Iron Nugget"),
      filterLabel("Cinnamon"), filterLabel("Peppercorn Sapling"), filterLabel("Peppercorn"), filterLabel("Pumpkin"),
      filterLabel("Cooking Oil"), filterLabel("Veggie Bait"), filterLabel("Soy Sauce"), filterLabel("Black Pepper"),
      filterLabel("Raw Tofeak"), filterLabel("Corned Beef"), filterLabel("Corned Beef Hash"), filterLabel("Impregnated Stick"),
      filterLabel("Redstone Reception Coil"), filterLabel("Silver Gear"), filterLabel("Hardened Upgrade Kit"), filterLabel("Reinforced Upgrade Kit"),
      filterLabel("Fused Quartz"), filterLabel("Block of Quartz"), filterLabel("Augment: Auxiliary Reception Coil"), filterName("mysticalagriculture:tier5_inferium_seeds"),
      filterLabel(u8"§aPrudentium Essence"), filterLabel(u8"§6Intermedium Essence"), filterLabel(u8"§bSuperium Essence"), filterLabel(u8"§cSupremium Essence"),
      filterLabel("Nickel Seeds"), filterLabel("Rock Crystal Ore"), filterLabel("Mystical Flower Seeds"), filterLabel("Mystical Flower Essence"),
      filterLabel("End Seeds"), filterLabel("End Essence"), filterLabel("Nether Seeds"), filterLabel("Nether Essence"),
      filterLabel("Uranium Ore Piece"), filterLabel("Uranium Ore"), filterLabel("Coal Seeds"), filterLabel("Coal Essence"),
      filterLabel("Redstone Seeds"), filterLabel("Redstone Essence"), filterLabel("Compressed Soul Sand"), filterLabel("Plain Yogurt"),
      filterLabel("Coconut Sapling"), filterLabel("Coconut"), filterLabel("Chili Pepper"), filterLabel("Garlic"),
      filterLabel("Mango Sapling"), filterLabel("Mango"), filterLabel("Chicken Curry"), filterLabel("Mango Chutney"),
      filterLabel("Dough"), filterLabel("Naan"), filterLabel("Black Iron Ingot"), filterLabel("Block of Invar"),
      filterLabel("Grains of Infinity"), filterLabel("Salis Mundus"), filterLabel("Quartz Sliver"), filterLabel("Terra Vis Crystal"),
      filterLabel("Ignis Vis Crystal"), filterLabel("Aqua Vis Crystal"), filterLabel("Electrum Gear"), filterLabel("Snowball"),
      filterLabel("Energetic Blend"), filterLabel("Energetic Alloy Ingot"), filterLabel("Vibrant Alloy Ingot"), filterLabel("Blizz Powder"),
      filterLabel("Cryotheum Dust"), filterLabel("Black Quartz"), filterLabel("Shibuichi Alloy"), filterLabel("Machine Chassis"),
      filterLabel("Advanced Plating"), filterLabel("Servomechanism"), filterLabel("Signalum Ingot"), filterLabel("Signalum Upgrade Kit"),
      filterLabel("Augment: Auxiliary Sieve"), filterLabel("Redstone Servo"), filterLabel("Tin Silver Alloy"), filterLabel("Lumium Ingot"),
      filterLabel("Lumium Gear"), filterLabel("Sulfur"), filterLabel("Sulfur Seeds"), filterLabel("Sulfur Essence"),
      filterLabel("Pyrotheum Dust"), filterLabel("Platinum Seeds"), filterLabel("Platinum Essence"), filterLabel("Platinum Ingot"),
      filterLabel("Lead Platinum Alloy"), filterLabel("Enderium Ingot"), filterName("minecraft:nether_brick"), filterLabel("Resonant Upgrade Kit"),
      filterLabel("Zombie Head"), filterLabel("Skeleton Skull"), filterLabel("Frog Leg"), filterLabel("Feather"),
      filterLabel("Dark Steel Ingot"), filterLabel("Pulsating Iron Ingot"), filterLabel("Pulsating Iron Nugget"), filterLabel("Pulsating Crystal"),
      filterLabel("Palis Crystal Block"), filterLabel("Palis Crystal"), filterLabel("Lapis Lazuli Dust"), filterLabel("Lapis Lazuli Plate"),
      filterLabel("Dense Lapis Lazuli Plate"), filterLabel("Sapphire"), filterLabelName("Slime Ball", "tconstruct:edible"), filterLabel("Congealed Blue Slime Block"),
      filterLabel("Empowered Palis Crystal"), filterLabel("Empowered Palis Crystal Block"), filterLabel("Modularium Alloy"), filterLabel("Redstone Gear"),
      filterLabel("Constantan Ingot"), filterLabel("Redstone Engineering Block")
    }));
    factory.addStorage(std::make_unique<StorageChest>(factory, "north", "334", Actions::up, Actions::east));
    factory.addBackup(filterLabel("Grains of Infinity"), 32);
    factory.addBackup(filterLabel("Mustard Seeds"), 32);
    factory.addBackup(filterLabel("Plain Yogurt"), 32);
    factory.addBackup(filterLabel("Chili Pepper"), 32);
    factory.addBackup(filterLabel("Spice Leaf"), 32);
    factory.addBackup(filterLabel("Peppercorn"), 32);
    factory.addBackup(filterLabel("Bellpepper"), 32);
    factory.addBackup(filterLabel("Soybean"), 32);
    factory.addBackup(filterLabel("Garlic"), 32);
    factory.addBackup(filterLabel("Ginger"), 32);
    factory.addBackup(filterLabel("Tomato"), 32);
    factory.addBackup(filterLabel("Onion"), 32);
    factory.addBackup(filterLabel("Seeds"), 32);
    // mobFarm dependencies: Rotten Flesh, String, Ender Pearl, Essence

    // reactor
    factory.addProcess(std::make_unique<ProcessReactorProportional>(factory, "reactor", "center"));

    // output
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "output", "center", "f98", Actions::south, Actions::west,
      std::vector<StockEntry>{}, INT_MAX, nullptr, outAll, std::vector<Recipe<int>>{}));

    // stock
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "stock", "center", "f98", Actions::up, Actions::west,
      std::vector<StockEntry>{
        {filterLabel("Tertius Alchemical Dust"), 16},
        {filterLabel("Uranium Ingot"), 16},
        {filterLabel("Seeds"), 16},
        {filterLabel("Birch Wood"), 16},
        {filterLabel("Blaze Powder Block"), 16},
        {filterLabel("Compressed Redstone"), 16},
        {filterLabel("Compressed Diamond"), 16},
        {filterLabel("Sand"), 16},
        {filterLabel("Cake"), 1},
        {filterLabel("Fluxed Phyto-Gro"), 16},
        {filterLabel("Redstone"), 16},
        {filterLabel("Glowstone Dust"), 16},
        {filterLabel("Ender Pearl"), 16},
        {filterLabel("Cryotheum Dust"), 16},
        {filterLabel("Aquamarine"), 16}
      }, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{}));

    // trash
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "trash", "north", "334", Actions::north, Actions::east,
      std::vector<StockEntry>{}, INT_MAX, nullptr, nullptr, std::vector<Recipe<int>>{
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
        {{}, {{filterLabel("Boron Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Boron Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Boron Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Boron Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Tough Alloy Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Tough Alloy Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Tough Alloy Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Tough Alloy Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Psimetal Exosuit Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Psimetal Exosuit Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Psimetal Exosuit Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Psimetal Exosuit Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Faraday Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Golden Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Diamond Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Diamond Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Diamond Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Diamond Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Chain Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manaweave Cowl"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manaweave Robe Top"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manaweave Robe Bottom"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manaweave Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manasteel Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manasteel Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manasteel Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Manasteel Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("NanoSuit Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel("NanoSuit Bodyarmor"), 1}}, INT_MAX},
        {{}, {{filterLabel("NanoSuit Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("NanoSuit Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Helm"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Hood"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Robe"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Greaves"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Crimson Cult Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§eInferium Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§eInferium Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§eInferium Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§eInferium Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§aPrudentium Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§aPrudentium Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§aPrudentium Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§aPrudentium Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§6Intermedium Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§6Intermedium Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§6Intermedium Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§6Intermedium Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§bSuperium Helmet"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§bSuperium Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§bSuperium Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel(u8"§bSuperium Boots"), 1}}, INT_MAX},
        {{}, {{filterLabel("Flux-Infused Helm"), 1}}, INT_MAX},
        {{}, {{filterLabel("Flux-Infused Chestplate"), 1}}, INT_MAX},
        {{}, {{filterLabel("Flux-Infused Leggings"), 1}}, INT_MAX},
        {{}, {{filterLabel("Flux-Infused Boots"), 1}}, INT_MAX},
      }));

    // phyto
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "phyto", "center", "1cb", Actions::south, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Mango"), 64}}, {{filterLabel("Mango Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Dye Essence"), 64}}, {{filterLabel("Dye Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Tin Essence"), 64}}, {{filterLabel("Tin Seeds"), 1}}, INT_MAX},
        {{{filterLabel("End Essence"), 64}}, {{filterLabel("End Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Coconut"), 64}}, {{filterLabel("Coconut Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Dirt Essence"), 64}}, {{filterLabel("Dirt Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Coal Essence"), 64}}, {{filterLabel("Coal Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Iron Essence"), 64}}, {{filterLabel("Iron Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Lead Essence"), 64}}, {{filterLabel("Lead Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Water Essence"), 64}}, {{filterLabel("Water Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Sulfur Essence"), 64}}, {{filterLabel("Sulfur Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nether Essence"), 64}}, {{filterLabel("Nether Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Copper Essence"), 64}}, {{filterLabel("Copper Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Silver Essence"), 64}}, {{filterLabel("Silver Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nature Essence"), 64}}, {{filterLabel("Nature Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Nickel Essence"), 64}}, {{filterLabel("Nickel Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Osmium Essence"), 64}}, {{filterLabel("Osmium Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Peppercorn"), 64}}, {{filterLabel("Peppercorn Sapling"), 1}}, INT_MAX},
        {{{filterLabel("Platinum Essence"), 64}}, {{filterLabel("Platinum Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Redstone Essence"), 64}}, {{filterLabel("Redstone Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Essence"), 64}}, {{filterLabel("Aluminum Seeds"), 1}}, INT_MAX},
        {{{filterLabel("Mystical Flower Essence"), 64}}, {{filterLabel("Mystical Flower Seeds"), 1}}, INT_MAX},
        {{{filterLabel(u8"§eInferium Essence"), 64}}, {{filterName("mysticalagriculture:tier5_inferium_seeds"), 1}}, INT_MAX}
      }));

    // platePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "platePress", "north", "0b8", Actions::north, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Lead Plate"), 64}}, {{filterLabel("Lead Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Plate"), 64}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Copper Plate"), 64}}, {{filterLabel("Copper Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Bronze Plate"), 64}}, {{filterLabel("Bronze Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Tin Plate"), 64}}, {{filterLabel("Tin Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Steel Plate"), 64}}, {{filterLabel("Steel Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Aluminum Plate"), 64}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Plate"), 64}}, {{filterLabel("Uranium Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Iron Large Plate"), 4}}, {{filterLabel("Block of Iron"), 1}}, INT_MAX}
      }));

    // arPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "arPress", "center", "12a", Actions::west, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Steel Rod"), 4}}, {{filterLabel("Steel Sheetmetal"), 1}}, INT_MAX},
        {{{filterLabel("Aluminium Rod"), 4}}, {{filterLabel("Aluminium Sheetmetal"), 1}}, INT_MAX}
      }));

    // gearPress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "gearPress", "north", "1e4", Actions::north, Actions::west,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Electrum Gear"), 16}}, {{filterLabel("Electrum Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Lumium Gear"), 16}}, {{filterLabel("Lumium Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Copper Gear"), 16}}, {{filterLabel("Copper Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Bronze Gear"), 16}}, {{filterLabel("Bronze Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Silver Gear"), 16}}, {{filterLabel("Silver Ingot"), 4}}, INT_MAX},
        {{{filterLabel("Gold Gear"), 16}}, {{filterLabel("Gold Ingot"), 4}}, INT_MAX}
      }));

    // wirePress
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "wirePress", "center", "5a0", Actions::up, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Aluminium Wire"), 64}}, {{filterLabel("Aluminum Ingot"), 1}}, INT_MAX}
      }));

    // pinkSlime
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "pinkSlime", "center", "12a", Actions::up, Actions::south,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Pink Slime"), 64}}, {{filterLabel("Green Slime Block"), 1}}, INT_MAX}
      }));

    // blueSlime
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "blueSlime", "north", "c3d", Actions::south, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabelName("Slime Ball", "tconstruct:edible"), 64}}, {{filterLabel("Green Slime Block"), 1}}, INT_MAX}
      }));

    // atomic
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "atomic", "center", "5a0", Actions::south, Actions::north,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Enori Crystal"), 64}}, {{filterLabel("Iron Ingot"), 1}}, INT_MAX},
        {{{filterLabel("Restonia Crystal"), 64}}, {{filterLabel("Redstone"), 1}}, INT_MAX},
        {{{filterLabel("Palis Crystal"), 64}}, {{filterLabel("Lapis Lazuli"), 1}}, INT_MAX},
        {{{filterLabel("Leather"), 64}}, {{filterLabel("Rotten Flesh"), 1}}, INT_MAX}
      }));

    // combustion
    factory.addProcess(std::make_unique<ProcessRedstoneConditional>(factory, "combustion", "center", "ed8", Actions::north, true, nullptr,
      [&](int value) { return value > 0; }, std::make_unique<ProcessSlotted>(factory, "combustion", "center", "f98", Actions::east, Actions::west,
      std::vector<size_t>{0, 1, 2, 3}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rock Crystal Ore"), 16}}, {
          {filterLabel("Stardust"), 10, {0}},
          {filterLabel("Diamond"), 3, {1}},
          {filterLabel("Aquamarine"), 10, {2}},
          {filterLabel("Stone"), 5, {3}}
        }, 60},
        {{{filterLabel("Grains of Infinity"), 64}}, {
          {filterLabel("Grains of Infinity"), 1, {0}, true},
          {filterLabel("Pulverized Obsidian"), 5, {1}},
          {filterLabel("Salis Mundus"), 1, {2}}
        }, 60},
        {{{filterLabel("Blaze Powder"), 64}}, {
          {filterLabel("Gunpowder"), 1, {0}}
        }, 16}
      })));

    // empower
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "empower", "north", "06f", Actions::up, Actions::south,
      std::vector<size_t>{0, 1, 2, 3, 4}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Empowered Palis Crystal Block"), 2}}, {
          {filterLabel("Palis Crystal Block"), 1, {0}},
          {filterLabel("Dense Lapis Lazuli Plate"), 1, {1}},
          {filterLabel("Cobalt Ingot"), 1, {2}},
          {filterLabel("Sapphire"), 1, {3}},
          {filterLabel("Congealed Blue Slime Block"), 1, {4}}
        }, 1}
      }));

    // cobbleGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "cobbleGen", "center", "258", Actions::east, Actions::down,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Cobblestone"), 256)));

    // obsidianGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "obsidianGen", "center", "5a0", Actions::west, Actions::north,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Obsidian"), 256)));

    // rubberGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "rubberGen", "north", "0b8", Actions::south, Actions::east,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Tiny Dry Rubber"), 256)));

    // waterGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "waterGen", "center", "1cb", Actions::up, Actions::north,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Fresh Water"), 256)));

    // snowballGen
    factory.addProcess(std::make_unique<ProcessInputless>(factory, "snowballGen", "center", "d40", Actions::south, Actions::up,
      0, ProcessInputless::makeNeeded(factory, filterLabel("Snowball"), 256)));

    // planter
    factory.addProcess(std::make_unique<ProcessScatteringWorkingSet>(factory, "planter", "center", "258", Actions::up, Actions::down,
      4, std::vector<size_t>{6, 7, 8, 9, 11, 12, 13, 14}, nullptr, std::vector<Recipe<>>{
        {{{filterLabel("Birch Wood"), 64}}, {{filterLabel("Birch Sapling"), 1}}},
        {{{filterLabel("Rice"), 64}}, {{filterLabel("Rice Seeds"), 1}}},
        {{{filterLabel("Mustard Seeds"), 64}}, {{filterLabel("Mustard Seeds"), 1, {}, true}}},
        {{{filterLabel("Chili Pepper"), 64}}, {{filterLabel("Chili Pepper"), 1, {}, true}}},
        {{{filterLabel("Bellpepper"), 64}}, {{filterLabel("Bellpepper"), 1, {}, true}}},
        {{{filterLabel("Spice Leaf"), 64}}, {{filterLabel("Spice Leaf"), 1, {}, true}}},
        {{{filterLabel("Soybean"), 64}}, {{filterLabel("Soybean"), 1, {}, true}}},
        {{{filterLabel("Garlic"), 64}}, {{filterLabel("Garlic"), 1, {}, true}}},
        {{{filterLabel("Tomato"), 64}}, {{filterLabel("Tomato"), 1, {}, true}}},
        {{{filterLabel("Ginger"), 64}}, {{filterLabel("Ginger"), 1, {}, true}}},
        {{{filterLabel("Onion"), 64}}, {{filterLabel("Onion"), 1, {}, true}}},
        {{{filterLabel("Seeds"), 64}}, {{filterLabel("Seeds"), 1, {}, true}}}
      }));

    // alchemicalA
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alchemicalA", "center", "441", Actions::up, Actions::east,
      std::vector<size_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Alchemical Gold Ingot"), 16}}, {
          {filterLabel("Glowstone Dust"), 3, {1}},
          {filterLabel("Gold Ingot"), 1, {3}}
        }, 9},
        {{{filterLabel("Tertius Alchemical Dust"), 16}}, {
          {filterLabel("Glowstone Dust"), 2, {1}},
          {filterLabel("Lapis Lazuli"), 2, {4}},
          {filterLabel("Alchemical Gold Ingot"), 1, {5}}
        }, 8},
        {{{filterLabel("Stardust"), 64}}, {
          {filterLabel("Aquamarine"), 2, {2}},
          {filterLabel("Iron Ingot"), 1, {6}},
          {filterLabel("Glowstone Dust"), 2, {1}}
        }, 8}
      }));

    // alchemicalB
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alchemicalB", "center", "441", Actions::north, Actions::east,
      std::vector<size_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Terra Vis Crystal"), 64}}, {
          {filterLabel("Quartz Sliver"), 9, {1}},
          {filterLabel("Dirt"), 1, {2}}
        }, 9},
        {{{filterLabel("Ignis Vis Crystal"), 64}}, {
          {filterLabel("Quartz Sliver"), 9, {1}},
          {filterLabel("Coal"), 1, {3}}
        }, 9},
        {{{filterLabel("Aqua Vis Crystal"), 64}}, {
          {filterLabel("Quartz Sliver"), 9, {1}},
          {filterName("minecraft:clay_ball"), 1, {4}}
        }, 9}
      }));

    // sandInduction
    factory.addProcess(std::make_unique<ProcessBuffered>(factory, "sandInduction", "center", "441", Actions::south, Actions::east,
      std::vector<StockEntry>{}, 16, nullptr, nullptr, std::vector<Recipe<int>>{
        {{{filterLabel("Rich Slag"), 64}}, {{filterLabel("Compass"), 1}}, INT_MAX},
        {{{filterLabel("Boron Ingot"), 64}}, {{filterLabel("Boron Ore"), 1}}, INT_MAX},
        {{{filterLabel("Magnesium Ingot"), 64}}, {{filterLabel("Magnesium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Lithium Ingot"), 64}}, {{filterLabel("Lithium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Cobalt Ingot"), 64}}, {{filterLabel("Cobalt Ore"), 1}}, INT_MAX},
        {{{filterLabel("Gold Ingot"), 64}}, {{filterLabel("Gold Ore"), 1}}, INT_MAX},
        {{{filterLabel("Ardite Ingot"), 64}}, {{filterLabel("Ardite Ore"), 1}}, INT_MAX},
        {{{filterLabel("Thorium Ingot"), 64}}, {{filterLabel("Thorium Ore"), 1}}, INT_MAX},
        {{{filterLabel("Uranium Ingot"), 64}}, {{filterLabel("Uranium Ore"), 1}}, INT_MAX}
      }));

    // manufactory
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "manufactory", "north", "06f", Actions::north, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Sand"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Niter"), 64}}, {{filterLabel("Sandstone"), 1, {0}}}, 16},
        {{{filterLabelName("Flour", "appliedenergistics2:material"), 64}}, {{filterLabel("Wheat"), 1, {0}}}, 16},
        {{{filterLabel("Silicon Ingot"), 64}}, {{filterLabel("Sand"), 1, {0}}}, 16},
        {{{filterLabel("Clay Dust"), 4}}, {{filterName("minecraft:clay"), 1, {0}}}, 16},
        {{{filterLabel("Lapis Lazuli Dust"), 16}}, {{filterLabel("Lapis Lazuli"), 1, {0}}}, 16}
      }));

    // pulverizer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "pulverizer", "north", "44f", Actions::west, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterName("harvestcraft:flouritem"), 64}}, {{filterLabel("Wheat"), 1, {0}}}, 16},
        {{{filterLabel("Black Pepper"), 64}}, {{filterLabel("Peppercorn"), 1, {0}}}, 16},
        {{{filterLabel("Ground Cinnamon"), 64}}, {{filterLabel("Cinnamon"), 1, {0}}}, 16},
        {{{filterLabel("Gravel"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Coal"), 64}}, {{filterLabel("Coal"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Charcoal"), 64}}, {{filterLabel("Charcoal"), 1, {0}}}, 16},
        {{{filterLabel("Pulverized Obsidian"), 64}}, {{filterLabel("Obsidian"), 1, {0}}}, 16}
      }));

    // xpTransposer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "xpTransposer", "north", "44f", Actions::south, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Blizz Powder"), 64}}, {{filterLabel("Snowball"), 2, {0}}}, 64}
      }));

    // induction
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "induction", "north", "44f", Actions::east, Actions::north,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Black Iron Ingot"), 64}}, {
          {filterLabel("Block of Invar"), 1, {0}},
          {filterLabel("Tough Alloy"), 1, {1}}
        }, 16},
        {{{filterName("minecraft:nether_brick"), 64}}, {
          {filterLabel("Soul Sand"), 1, {0}},
          {filterLabel("Netherrack"), 2, {1}}
        }, 16}
      }));

    // cinnamon
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "cinnamon", "center", "d40", Actions::east, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Cinnamon"), 64}}, {{filterLabel("Plant Matter"), 1, {0}}}, 16}
      }));

    // signalum
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "signalum", "center", "258", Actions::south, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Signalum Ingot"), 64}}, {{filterLabel("Shibuichi Alloy"), 1, {0}}}, 16}
      }));

    // lumium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "lumium", "center", "f98", Actions::north, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Lumium Ingot"), 64}}, {{filterLabel("Tin Silver Alloy"), 1, {0}}}, 16}
      }));

    // enderium
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "enderium", "north", "44f", Actions::up, Actions::north,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Enderium Ingot"), 64}}, {{filterLabel("Lead Platinum Alloy"), 1, {0}}}, 16}
      }));

    // furnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "furnace", "center", "d40", Actions::down, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Glass"), 64}}, {{filterLabel("Sand"), 1, {0}}}, 16},
        {{{filterLabel("Stone"), 64}}, {{filterLabel("Cobblestone"), 1, {0}}}, 16},
        {{{filterLabel("Bread"), 64}}, {{filterLabelName("Flour", "appliedenergistics2:material"), 1, {0}}}, 16},
        {{{filterLabel("Charcoal"), 64}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16},
        {{{filterLabel("Graphite Ingot"), 64}}, {{filterLabel("Charcoal"), 1, {0}}}, 16},
        {{{filterLabel("Plastic"), 64}}, {{filterLabel("Dry Rubber"), 1, {0}}}, 16},
        {{{filterLabel("Brick"), 64}}, {{filterName("minecraft:clay_ball"), 1, {0}}}, 16},
        {{{filterLabel("Cactus Green"), 64}}, {{filterLabel("Cactus"), 1, {0}}}, 16},
        {{{filterLabel("Printed Circuit Board (PCB)"), 16}}, {{filterLabel("Raw Circuit Board"), 1, {0}}}, 16}
      }));

    // alloyFurnace
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "alloyFurnace", "north", "ab7", Actions::east, Actions::west,
      std::vector<size_t>{0, 1}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Bronze Ingot"), 64}}, {
          {filterLabel("Copper Ingot"), 3, {0}},
          {filterLabel("Tin Ingot"), 1, {1}}
        }, 18},
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
        }, 16},
        {{{filterLabel("Ferroboron Alloy"), 64}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Boron Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Tough Alloy"), 64}}, {
          {filterLabel("Ferroboron Alloy"), 1, {0}},
          {filterLabel("Lithium Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Fused Quartz"), 64}}, {
          {filterLabel("Nether Quartz"), 4, {0}},
          {filterLabel("Block of Quartz"), 1, {1}}
        }, 16},
        {{{filterLabel("Energetic Alloy Ingot"), 64}}, {
          {filterLabel("Gold Ingot"), 1, {0}},
          {filterLabel("Energetic Blend"), 2, {1}}
        }, 16},
        {{{filterLabel("Vibrant Alloy Ingot"), 64}}, {
          {filterLabel("Ender Pearl"), 1, {0}},
          {filterLabel("Energetic Alloy Ingot"), 1, {1}}
        }, 16},
        {{{filterLabel("Shibuichi Alloy"), 64}}, {
          {filterLabel("Silver Ingot"), 1, {0}},
          {filterLabel("Copper Ingot"), 3, {1}}
        }, 18},
        {{{filterLabel("Tin Silver Alloy"), 64}}, {
          {filterLabel("Silver Ingot"), 1, {0}},
          {filterLabel("Tin Ingot"), 3, {1}}
        }, 18},
        {{{filterLabel("Lead Platinum Alloy"), 64}}, {
          {filterLabel("Platinum Ingot"), 1, {0}},
          {filterLabel("Lead Ingot"), 3, {1}}
        }, 18},
        {{{filterLabel("Dark Steel Ingot"), 64}}, {
          {filterLabel("Steel Ingot"), 1, {0}},
          {filterLabel("Obsidian"), 1, {1}}
        }, 18},
        {{{filterLabel("Pulsating Iron Ingot"), 64}}, {
          {filterLabel("Iron Ingot"), 1, {0}},
          {filterLabel("Ender Pearl"), 1, {1}}
        }, 16},
        {{{filterLabel("Constantan Ingot"), 64}}, {
          {filterLabel("Copper Ingot"), 1, {0}},
          {filterLabel("Nickel Ingot"), 1, {1}}
        }, 16}
      }));

    // slimeCrafter
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "slimeCrafter", "north", "334", Actions::south, Actions::east,
      std::vector<size_t>{1, 3, 5, 7}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Rice Slimeball"), 64}}, {
          {filterLabel("Rice Dough"), 4, {1, 3, 5, 7}}
        }, 16}
      }));

    // treatedWood
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "treatedWood", "center", "664", Actions::up, Actions::south,
      std::vector<size_t>{0, 1, 2, 3, 5, 6, 7, 8}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Treated Wood Planks"), 64}}, {
          {filterLabel("Birch Wood Planks"), 8, {0, 1, 2, 3, 5, 6, 7, 8}}
        }, 8}
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
        {{{filterLabel("Basic Control Circuit"), 64}}, {{filterLabel("Osmium Ingot"), 1, {0}}}, 16},
        {{{filterLabel("Redstone Reception Coil"), 16}}, {{filterLabel("Gold Ingot"), 1, {0}}}, 16}
      }));

    // diamondInfusion
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondInfusion", "center", "664", Actions::down, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Reinforced Alloy"), 64}}, {{filterLabel("Enriched Alloy"), 1, {0}}}, 16}
      }));

    // centrifuge
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "centrifuge", "north", "1e4", Actions::down, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Silken Tofu"), 64}}, {{filterLabel("Soybean"), 1, {0}}}, 64},
        {{{filterLabel("Soy Milk"), 64}}, {{filterLabel("Silken Tofu"), 1, {0}}}, 64},
        {{{filterLabel("Cooking Oil"), 64}}, {{filterLabel("Pumpkin"), 1, {0}}}, 64},
        {{{filterLabel("Sugar"), 64}}, {{filterLabel("Sugar Canes"), 1, {0}}}, 64}
      }));

    // compressedHammer
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressedHammer", "north", "1e4", Actions::east, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterName("exnihilocreatio:block_netherrack_crushed"), 64}}, {{filterLabel("Compressed Netherrack"), 1, {0}}}, 16},
        {{{filterLabel("Crushed Endstone"), 64}}, {{filterLabel("Compressed End Stone"), 1, {0}}}, 16}
      }));

    // diamondSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "diamondSieve", "north", "1e4", Actions::south, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Boron Ore Piece"), 64},
          {filterLabel("Magnesium Ore Piece"), 64},
          {filterLabel("Lithium Ore Piece"), 64},
          {filterLabel("Cobalt Ore Piece"), 64},
          {filterLabel("Gold Ore Piece"), 64},
          {filterLabel("Ardite Ore Piece"), 64},
          {filterLabel("Thorium Ore Piece"), 64}
        }, {{filterLabel("Compressed Nether Gravel"), 1, {0}}}, 16},
        {{{filterLabel("Nether Quartz"), 64}}, {{filterLabel("Compressed Soul Sand"), 1, {0}}}, 16}
      }));

    // ironSieve
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "ironSieve", "north", "06f", Actions::west, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Glowstone Dust"), 64}}, {{filterLabel("Compressed Soul Sand"), 1, {0}}}, 16},
        {{{filterLabel("Uranium Ore Piece"), 64}}, {{filterLabel("Compressed Ender Gravel"), 1, {0}}}, 16}
      }));

    // impregnatedStick
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "impregnatedStick", "north", "334", Actions::west, Actions::east,
      std::vector<size_t>{16}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Impregnated Stick"), 64}}, {{filterLabel("Birch Wood"), 2, {16}}}, 16}
      }));

    // tinElectronTube
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "tinElectronTube", "center", "664", Actions::west, Actions::south,
      std::vector<size_t>{16, 17}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Tin Electron Tube"), 64}}, {
          {filterLabel("Tin Ingot"), 5, {16}},
          {filterLabel("Redstone"), 2, {17}}
        }, 60}
      }));

    // teFrame
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "teFrame", "center", "1cb", Actions::west, Actions::north,
      std::vector<size_t>{11, 12, 13, 14, 15, 16}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabelName("Machine Frame", "thermalexpansion:frame"), 4}}, {
          {filterName("rftools:machine_frame"), 1, {11}},
          {filterLabel("Device Frame"), 1, {12}},
          {filterLabel("Iron Casing"), 1, {13}},
          {filterLabel("Machine Case"), 1, {14}},
          {filterLabel("Heavy Engineering Block"), 1, {15}},
          {filterLabel("Enori Crystal"), 4, {16}}
        }, 16}
      }));

    // hardenedCasing
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "hardenedCasing", "center", "664", Actions::north, Actions::south,
      std::vector<size_t>{16, 17}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Hardened Casing"), 4}}, {
          {filterLabel("Sturdy Casing"), 1, {16}},
          {filterLabel("Diamond"), 4, {17}}
        }, 16}
      }));

    // compressor
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "compressor", "north", "ab7", Actions::north, Actions::west,
      std::vector<size_t>{6}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Advanced Alloy"), 4}}, {{filterLabel("Mixed Metal Ingot"), 1, {6}}}, 16},
        {{{filterLabel("Carbon Plate"), 4}}, {{filterLabel("Raw Carbon Mesh"), 1, {6}}}, 16},
        {{{filterLabel("Lapis Lazuli Plate"), 16}}, {{filterLabel("Lapis Lazuli Dust"), 1, {6}}}, 16},
        {{{filterLabel("Dense Lapis Lazuli Plate"), 2}}, {{filterLabel("Lapis Lazuli Plate"), 9, {6}}}, 18}
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
        {{{filterLabel("Tin Item Casing"), 4}}, {{filterLabel("Tin Plate"), 1, {6}}}, 16},
        {{{filterLabel("Lead Item Casing"), 4}}, {{filterLabel("Lead Plate"), 1, {6}}}, 16},
        {{{filterLabel("Copper Item Casing"), 4}}, {{filterLabel("Copper Plate"), 1, {6}}}, 16}
      }));

    // rockCrusher
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCrusher", "center", "d40", Actions::north, Actions::up,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{
          {filterLabel("Dirty Lapis Lazuli"), 16},
          {filterLabel("Dirty Certus Quartz"), 16},
          {filterLabel("Dirty Black Quartz"), 16},
          {filterLabel("Dirty Diamond"), 16},
          {filterLabel("Dirty Emerald"), 16},
          {filterLabel("Dirty Sapphire"), 16},
          {filterLabel("Dirty Aquamarine"), 16}
        }, {{filterLabel("Stone"), 1, {0}}}, 16}
      }));

    // rockCleaner
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "rockCleaner", "north", "ab7", Actions::south, Actions::west,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Certus Quartz Crystal"), 64}}, {{filterLabel("Dirty Certus Quartz"), 1, {0}}}, 16},
        {{{filterLabel("Black Quartz"), 64}}, {{filterLabel("Dirty Black Quartz"), 1, {0}}}, 16},
        {{{filterLabel("Lapis Lazuli"), 256}}, {{filterLabel("Dirty Lapis Lazuli"), 1, {0}}}, 16},
        {{{filterLabel("Diamond"), 64}}, {{filterLabel("Dirty Diamond"), 1, {0}}}, 16},
        {{{filterLabel("Emerald"), 64}}, {{filterLabel("Dirty Emerald"), 1, {0}}}, 16},
        {{{filterLabel("Sapphire"), 64}}, {{filterLabel("Dirty Sapphire"), 1, {0}}}, 16},
        {{{filterLabel("Aquamarine"), 64}}, {{filterLabel("Dirty Aquamarine"), 1, {0}}}, 16},
        {{}, {{filterLabel("Dirty Coal"), 1, {0}}}, 16}
      }));

    // sawmill
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "sawmill", "center", "258", Actions::north, Actions::down,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Birch Wood Planks"), 64}}, {{filterLabel("Birch Wood"), 1, {0}}}, 16},
        {{{filterLabel("Stick"), 64}}, {{filterLabel("Birch Wood Planks"), 1, {0}}}, 16}
      }));

    // charger
    factory.addProcess(std::make_unique<ProcessSlotted>(factory, "charger", "north", "06f", Actions::east, Actions::south,
      std::vector<size_t>{0}, nullptr, std::vector<Recipe<int, std::vector<size_t>>>{
        {{{filterLabel("Fluxed Phyto-Gro"), 64}}, {{filterLabel("Rich Phyto-Gro"), 1, {0}}}, 64}
      }));

    // workbench
    factory.addProcess(std::make_unique<ProcessRFToolsControlWorkbench>(factory, "workbench", "north", "0b8", "ab7", Actions::east, Actions::west, Actions::west,
      std::vector<Recipe<std::pair<int, std::vector<NonConsumableInfo>>, std::vector<size_t>>>{
        {{{filterLabel("Plant Matter"), 64}}, {
          {filterLabel("Wheat"), 5, {5, 2, 4, 6, 8}}
        }, {12, {}}},
        {{{filterLabel("Congealed Blue Slime Block"), 2}}, {
          {filterLabelName("Slime Ball", "tconstruct:edible"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Dirt"), 64}}, {
          {filterLabel("Dirt Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {2, {}}},
        {{{filterLabel("Redstone"), 64}}, {
          {filterLabel("Redstone Essence"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {4, {}}},
        {{{filterLabel("Button"), 64}}, {
          {filterLabel("Stone"), 1, {5}}
        }, {64, {}}},
        {{{filterLabel("Wheat"), 64}}, {
          {filterLabel("Nature Essence"), 3, {1, 2, 3}}
        }, {5, {}}},
        {{{filterLabel("Potato"), 64}}, {
          {filterLabel("Nature Essence"), 3, {1, 5, 3}}
        }, {5, {}}},
        {{{filterLabel("Sugar Canes"), 64}}, {
          {filterLabel("Nature Essence"), 6, {2, 4, 5, 6, 7, 9}}
        }, {4, {}}},
        {{{filterLabel("Netherrack"), 64}}, {
          {filterLabel("Nether Essence"), 5, {5, 2, 4, 6, 8}}
        }, {2, {}}},
        {{{filterLabel("Cactus"), 64}}, {
          {filterLabel("Nature Essence"), 7, {1, 2, 3, 5, 7, 8, 9}}
        }, {4, {}}},
        {{{filterLabel("Pumpkin"), 64}}, {
          {filterLabel("Nature Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {4, {}}},
        {{{filterLabel("Aluminum Ingot"), 64}}, {
          {filterLabel("Aluminum Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Iron Ingot"), 64}}, {
          {filterLabel("Iron Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Copper Ingot"), 64}}, {
          {filterLabel("Copper Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Platinum Ingot"), 64}}, {
          {filterLabel("Platinum Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Silver Ingot"), 64}}, {
          {filterLabel("Silver Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("End Stone"), 64}}, {
          {filterLabel("End Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
        {{{filterLabel("Soul Sand"), 64}}, {
          {filterLabel("Nether Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {3, {}}},
        {{{filterLabel("Coal"), 64}}, {
          {filterLabel("Coal Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {5, {}}},
        {{{filterLabel("Nickel Ingot"), 64}}, {
          {filterLabel("Nickel Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Osmium Ingot"), 64}}, {
          {filterLabel("Osmium Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Lead Ingot"), 64}}, {
          {filterLabel("Lead Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Tin Ingot"), 64}}, {
          {filterLabel("Tin Essence"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterName("minecraft:red_mushroom"), 64}}, {
          {filterLabel("Nature Essence"), 2, {2, 8}},
          {filterLabel("Dirt Essence"), 1, {5}}
        }, {8, {}}},
        {{{filterName("minecraft:brown_mushroom"), 64}}, {
          {filterLabel("Nature Essence"), 2, {1, 3}},
          {filterLabel("Dirt Essence"), 1, {2}}
        }, {8, {}}},
        {{{filterLabel("Energetic Blend"), 64}}, {
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Glowstone Dust"), 1, {2}}
        }, {32, {}}},
        {{{filterLabel("Rich Phyto-Gro"), 64}}, {
          {filterLabel("Pulverized Charcoal"), 1, {1}},
          {filterLabel("Niter"), 1, {2}},
          {filterLabel("Rich Slag"), 1, {3}}
        }, {4, {}}},
        {{{filterLabel("Plain Yogurt"), 64}}, {
          {filterLabel("Plain Yogurt"), 1, {1}, true},
          {filterLabel("Leather"), 1, {2}}
        }, {16, {}}},
        {{{filterLabel("Sandstone"), 64}}, {
          {filterLabel("Sand"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Block of Quartz"), 64}}, {
          {filterLabel("Nether Quartz"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Boron Ore"), 64}}, {
          {filterLabel("Boron Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Magnesium Ore"), 64}}, {
          {filterLabel("Magnesium Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Uranium Ore"), 64}}, {
          {filterLabel("Uranium Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Lithium Ore"), 64}}, {
          {filterLabel("Lithium Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Cobalt Ore"), 64}}, {
          {filterLabel("Cobalt Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Gold Ore"), 64}}, {
          {filterLabel("Gold Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Ardite Ore"), 64}}, {
          {filterLabel("Ardite Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Thorium Ore"), 64}}, {
          {filterLabel("Thorium Ore Piece"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterName("minecraft:clay"), 64}}, {
          {filterName("minecraft:clay_ball"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterName("minecraft:clay_ball"), 64}}, {
          {filterLabel("Dirt Essence"), 2, {2, 4}},
          {filterLabel("Water Essence"), 2, {1, 5}}
        }, {4, {}}},
        {{{filterLabel("Slimeball"), 64}}, {
          {filterLabel("Green Slime Block"), 1, {5}}
        }, {7, {}}},
        {{{filterLabel("Block of Redstone"), 64}}, {
          {filterLabel("Redstone"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Paper"), 64}}, {
          {filterLabel("Rice"), 3, {1, 5, 9}}
        }, {21, {}}},
        {{{filterLabel("Rice Dough"), 64}}, {
          {filterLabel("Rice"), 3, {1, 2, 4}}
        }, {21, {}}},
        {{{filterLabel("Blaze Powder Block"), 64}}, {
          {filterLabel("Blaze Powder"), 4, {1, 2, 4, 5}}
        }, {16, {}}},
        {{{filterLabel("Dry Rubber"), 64}}, {
          {filterLabel("Tiny Dry Rubber"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Block of Iron"), 64}}, {
          {filterLabel("Iron Ingot"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Palis Crystal Block"), 2}}, {
          {filterLabel("Palis Crystal"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Netherrack"), 64}}, {
          {filterLabel("Netherrack"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Ender Gravel"), 64}}, {
          {filterLabel("Crushed Endstone"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed End Stone"), 64}}, {
          {filterLabel("End Stone"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Graphite Block"), 64}}, {
          {filterLabel("Graphite Ingot"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Block of Invar"), 64}}, {
          {filterLabel("Invar Ingot"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Nether Gravel"), 64}}, {
          {filterName("exnihilocreatio:block_netherrack_crushed"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Green Slime Block"), 64}}, {
          {filterLabel("Rice Slimeball"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Cobblestone"), 64}}, {
          {filterLabel("Cobblestone"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Compressed Soul Sand"), 64}}, {
          {filterLabel("Soul Sand"), 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}}
        }, {7, {}}},
        {{{filterLabel("Glass Pane"), 64}}, {
          {filterLabel("Glass"), 6, {1, 2, 3, 4, 5, 6}}
        }, {4, {}}},
        {{{filterLabel("Dandelion Yellow"), 64}}, {
          {filterLabel("Dye Essence"), 3, {1, 3, 5}}
        }, {16, {}}},
        {{{filterLabel("Sulfur"), 64}}, {
          {filterLabel("Sulfur Essence"), 3, {1, 2, 4}}
        }, {8, {}}},
        {{{filterLabel("Book"), 64}}, {
          {filterLabel("Paper"), 3, {1, 2, 4}},
          {filterLabel("Leather"), 1, {5}}
        }, {21, {}}},
        {{{filterLabel("Compass"), 64}}, {
          {filterLabel("Iron Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Torch"), 64}}, {
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Stick"), 1, {4}}
        }, {64, {}}},
        {{{filterLabel("Gold Nugget"), 64}}, {
          {filterLabel("Gold Ingot"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Empowered Palis Crystal"), 64}}, {
          {filterLabel("Empowered Palis Crystal Block"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Quartz Sliver"), 64}}, {
          {filterLabel("Nether Quartz"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Iron Nugget"), 64}}, {
          {filterLabel("Iron Ingot"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Pulsating Iron Nugget"), 64}}, {
          {filterLabel("Pulsating Iron Ingot"), 1, {1}}
        }, {7, {}}},
        {{{filterLabel("Raw Tofeeg"), 64}}, {
          {filterLabel("Firm Tofu"), 1, {1}},
          {filterLabel("Dandelion Yellow"), 1, {2}}
        }, {32, {{1, 3}}}},
        {{{filterLabel("Raw Toficken"), 64}}, {
          {filterLabel("Firm Tofu"), 1, {2}},
          {filterName("harvestcraft:flouritem"), 1, {3}},
          {filterLabel("Cooking Oil"), 1, {4}},
          {filterLabel("Spice Leaf"), 1, {5}}
        }, {16, {{1, 1}}}},
        {{{filterLabel("Ketchup"), 64}}, {
          {filterLabel("Tomato"), 1, {1}}
        }, {64, {{2, 2}}}},
        {{{filterLabel("Salt"), 64}}, {
          {filterLabel("Fresh Water"), 1, {1}}
        }, {64, {{3, 2}}}},
        {{{filterLabel("Butter"), 64}}, {
          {filterLabel("Silken Tofu"), 1, {1}},
          {filterLabel("Salt"), 1, {2}}
        }, {64, {{4, 3}}}},
        {{{filterLabel("Toast"), 64}}, {
          {filterLabel("Bread"), 1, {1}},
          {filterLabel("Butter"), 1, {2}}
        }, {64, {{5, 3}}}},
        {{{filterLabel("Cheese"), 64}}, {
          {filterLabel("Soy Milk"), 1, {1}},
          {filterLabel("Salt"), 1, {2}}
        }, {64, {{3, 3}}}},
        {{{filterLabel("Chicken Curry"), 64}}, {
          {filterLabel("Coconut"), 1, {2}},
          {filterLabel("Plain Yogurt"), 1, {3}},
          {filterLabel("Raw Toficken"), 1, {4}},
          {filterLabel("Ginger"), 1, {5}},
          {filterLabel("Chili Pepper"), 1, {6}},
          {filterLabel("Rice"), 1, {7}},
          {filterLabel("Ground Cinnamon"), 1, {8}},
          {filterLabel("Garlic"), 1, {9}}
        }, {64, {{3, 1}}}},
        {{{filterLabel("Deluxe Chicken Curry"), 64}}, {
          {filterLabel("Chicken Curry"), 1, {2}},
          {filterLabel("Rice"), 1, {3}},
          {filterLabel("Naan"), 1, {4}},
          {filterLabel("Mango Chutney"), 1, {5}}
        }, {64, {{1, 1}}}},
        {{{filterLabel("Mango Chutney"), 64}}, {
          {filterLabel("Mango"), 1, {2}},
          {filterLabel("Spice Leaf"), 1, {3}},
          {filterLabel("Mustard Seeds"), 1, {4}},
          {filterLabel("Cooking Oil"), 1, {5}}
        }, {64, {{3, 1}}}},
        {{{filterLabel("Naan"), 64}}, {
          {filterLabel("Dough"), 1, {2}},
          {filterLabel("Onion"), 1, {4}},
          {filterLabel("Cooking Oil"), 1, {5}},
        }, {64, {{7, 1}}}},
        {{{filterLabel("Dough"), 64}}, {
          {filterLabel("Fresh Water"), 1, {2}},
          {filterName("harvestcraft:flouritem"), 1, {4}},
          {filterLabel("Salt"), 1, {5}},
        }, {64, {{8, 1}}}},
        {{{filterLabel("Soy Sauce"), 64}}, {
          {filterLabel("Soybean"), 1, {2}},
          {filterLabel("Fresh Water"), 1, {4}},
          {filterLabel("Salt"), 1, {5}}
        }, {64, {{2, 1}}}},
        {{{filterLabel("Raw Tofeak"), 64}}, {
          {filterLabel("Firm Tofu"), 1, {1}},
          {filterLabel("Mushroom"), 1, {2}},
          {filterLabel("Soy Sauce"), 1, {3}},
          {filterLabel("Black Pepper"), 1, {4}},
          {filterLabel("Cooking Oil"), 1, {5}}
        }, {10, {{1, 6}}}},
        {{{filterLabel("Corned Beef"), 64}}, {
          {filterLabel("Raw Tofeak"), 1, {2}},
          {filterLabel("Salt"), 1, {3}},
          {filterLabel("Sugar"), 1, {4}},
          {filterLabel("Cinnamon"), 1, {5}},
          {filterLabel("Mustard Seeds"), 1, {6}},
          {filterLabel("Peppercorn"), 1, {7}},
          {filterLabel("Spice Leaf"), 1, {8}},
          {filterLabel("Ginger"), 1, {9}},
        }, {64, {{3, 1}}}},
        {{{filterLabel("Corned Beef Hash"), 64}}, {
          {filterLabel("Corned Beef"), 1, {2}},
          {filterLabel("Onion"), 1, {3}},
          {filterLabel("Bellpepper"), 1, {4}},
          {filterLabel("Potato"), 1, {5}},
          {filterLabel("Raw Tofeeg"), 1, {6}},
          {filterLabel("Butter"), 1, {7}},
          {filterLabel("Cheese"), 1, {8}}
        }, {64, {{7, 1}}}},
        {{{filterLabel("Corned Beef Breakfast"), 64}}, {
          {filterLabel("Corned Beef Hash"), 1, {2}},
          {filterLabel("Raw Tofeeg"), 1, {3}},
          {filterLabel("Toast"), 1, {4}},
          {filterLabel("Ketchup"), 1, {5}},
          {filterLabel("Soy Milk"), 1, {6}}
        }, {64, {{7, 1}}}},
        {{{filterLabel("Salis Mundus"), 64}}, {
          {filterLabel("Redstone"), 1, {1}},
          {filterLabel("Terra Vis Crystal"), 1, {2}},
          {filterLabel("Ignis Vis Crystal"), 1, {3}},
          {filterLabel("Aqua Vis Crystal"), 1, {4}}
        }, {64, {{9, 9}, {10, 8}}}},
        {{{filterLabel(u8"§aPrudentium Essence"), 64}}, {
          {filterLabel(u8"§eInferium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{6, 5}}}},
        {{{filterLabel(u8"§6Intermedium Essence"), 64}}, {
          {filterLabel(u8"§aPrudentium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{6, 5}}}},
        {{{filterLabel(u8"§bSuperium Essence"), 64}}, {
          {filterLabel(u8"§6Intermedium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{6, 5}}}},
        {{{filterLabel(u8"§cSupremium Essence"), 64}}, {
          {filterLabel(u8"§bSuperium Essence"), 4, {2, 4, 6, 8}}
        }, {16, {{6, 5}}}},
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
        {{{filterLabel("Dropper"), 16}}, {
          {filterLabel("Cobblestone"), 7, {1, 2, 3, 4, 6, 7, 9}},
          {filterLabel("Redstone"), 1, {8}}
        }, {9, {}}},
        {{{filterLabel("Dispenser"), 16}}, {
          {filterLabel("Cobblestone"), 7, {1, 2, 3, 4, 6, 7, 9}},
          {filterLabel("String"), 1, {5}},
          {filterLabel("Redstone"), 1, {8}}
        }, {9, {}}},
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
        {{{filterLabel("Machine Chassis"), 4}}, {
          {filterLabel("Steel Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Lead Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Tough Alloy"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Bucket"), 16}}, {
          {filterLabel("Iron Plate"), 3, {1, 3, 5}}
        }, {21, {}}},
        {{{filterLabel("Iron Sheetmetal"), 4}}, {
          {filterLabel("Iron Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Lead Sheetmetal"), 4}}, {
          {filterLabel("Lead Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Steel Sheetmetal"), 4}}, {
          {filterLabel("Steel Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Aluminium Sheetmetal"), 4}}, {
          {filterLabel("Aluminum Plate"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Iron Mechanical Component"), 4}}, {
          {filterLabel("Iron Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Cryotheum Dust"), 64}}, {
          {filterLabel("Blizz Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {4}},
          {filterLabel("Snowball"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Pyrotheum Dust"), 64}}, {
          {filterLabel("Blaze Powder"), 2, {1, 2}},
          {filterLabel("Redstone"), 1, {4}},
          {filterLabel("Sulfur"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Coil"), 4}}, {
          {filterLabel("Iron Ingot"), 1, {5}},
          {filterLabel("Copper Cable"), 8, {1, 2, 3, 4, 6, 7, 8, 9}}
        }, {8, {}}},
        {{{filterLabel("Sturdy Casing"), 4}}, {
          {filterLabel("Copper Gear"), 2, {1, 3}},
          {filterLabel("Bronze Gear"), 2, {7, 9}},
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}}
        }, {16, {}}},
        {{{filterLabel("Iron Bars"), 4}}, {
          {filterLabel("Iron Ingot"), 6, {4, 5, 6, 7, 8, 9}}
        }, {4, {}}},
        {{{filterLabel("Range Addon"), 2}}, {
          {filterLabel("Cobblestone"), 6, {1, 3, 4, 6, 7, 9}},
          {filterLabel("Plastic"), 2, {2, 8}},
          {filterLabel("Glass Pane"), 1, {5}}
        }, {1, {}}},
        {{{filterLabel("Electric Motor"), 4}}, {
          {filterLabel("Tin Item Casing"), 2, {2, 8}},
          {filterLabel("Coil"), 2, {4, 6}},
          {filterLabel("Iron Ingot"), 1, {5}}
        }, {32, {}}},
        {{{filterLabel("Heat Vent"), 4}}, {
          {filterLabel("Iron Bars"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Plate"), 4, {2, 4, 6, 8}},
          {filterLabel("Electric Motor"), 1, {5}}
        }, {16, {}}},
        {{{filterName("rftools:machine_frame"), 4}}, {
          {filterLabel("Heat Vent"), 2, {1, 3}},
          {filterLabel("Gold Gear"), 1, {2}},
          {filterLabel("Dry Rubber"), 2, {4, 6}},
          {filterLabel("Machine Case"), 1, {5}},
          {filterLabel("Pink Slime"), 2, {7, 9}},
          {filterLabel("Range Addon"), 1, {8}}
        }, {1, {}}},
        {{{filterLabel("Steel Scaffolding"), 4}}, {
          {filterLabel("Steel Ingot"), 3, {1, 2, 3}},
          {filterLabel("Steel Rod"), 3, {5, 7, 9}}
        }, {10, {}}},
        {{{filterLabel("Heavy Engineering Block"), 4}}, {
          {filterLabel("Uranium Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Iron Mechanical Component"), 2, {4, 6}},
          {filterLabel("Reinforced Alloy"), 2, {2, 8}},
          {filterLabel("Steel Scaffolding"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Iron Casing"), 4}}, {
          {filterLabel("Iron Sheetmetal"), 4, {1, 3, 7, 9}},
          {filterLabel("Tin Electron Tube"), 4, {2, 4, 6, 8}},
          {filterLabel("Hardened Casing"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Cake"), 2}}, {
          {filterLabel("Wheat"), 3, {7, 8, 9}},
          {filterLabel("Raw Tofeeg"), 1, {5}},
          {filterLabel("Sugar"), 2, {4, 6}},
          {filterLabel("Soy Milk"), 3, {1, 2, 3}}
        }, {1, {}}},
        {{{filterLabel("Piston"), 64}}, {
          {filterLabel("Treated Wood Planks"), 3, {1, 2, 3}},
          {filterLabel("Iron Plate"), 1, {5}},
          {filterLabel("Compressed Cobblestone"), 4, {4, 6, 7, 9}},
          {filterLabel("Redstone"), 1, {8}}
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
        {{{filterLabel("Servomechanism"), 16}}, {
          {filterLabel("Ferroboron Alloy"), 2, {1, 3}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Steel Ingot"), 3, {5, 7, 9}},
          {filterLabel("Copper Ingot"), 1, {8}}
        }, {21, {}}},
        {{{filterLabel("Copper Solenoid"), 16}}, {
          {filterLabel("Copper Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Copper Item Casing"), 2, {2, 8}},
          {filterLabel("Aluminium Rod"), 2, {4, 6}},
          {filterLabel("Mixed Metal Ingot"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Chest"), 16}}, {
          {filterLabel("Birch Wood"), 4, {1, 3, 7, 9}},
          {filterLabel("Treated Wood Planks"), 4, {2, 4, 6, 8}},
          {filterLabel("Button"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Hopper"), 16}}, {
          {filterLabel("Iron Plate"), 5, {1, 3, 4, 6, 8}},
          {filterLabel("Chest"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Raw Circuit Board"), 16}}, {
          {filterLabel("Gold Ingot"), 1, {1}},
          {filterName("minecraft:clay_ball"), 1, {2}},
          {filterLabel("Cactus Green"), 1, {4}}
        }, {8, {}}},
        {{{filterLabel("Transistor"), 16}}, {
          {filterLabel("Iron Ingot"), 3, {1, 2, 3}},
          {filterLabel("Paper"), 1, {5}},
          {filterLabel("Redstone"), 1, {8}},
          {filterLabel("Gold Nugget"), 2, {4, 6}}
        }, {8, {}}},
        {{{filterLabel("Analyzer"), 16}}, {
          {filterLabel("Redstone Torch"), 1, {1}},
          {filterLabel("Transistor"), 1, {4}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {7}},
          {filterLabel("Gold Nugget"), 2, {5, 8}}
        }, {32, {}}},
        {{{filterLabel("Inventory Controller Upgrade"), 16}}, {
          {filterLabel("Gold Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Analyzer"), 1, {2}},
          {filterLabel("Dropper"), 1, {4}},
          {filterLabel("Microchip (Tier 2)"), 1, {5}},
          {filterLabel("Piston"), 1, {6}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Tank Controller Upgrade"), 16}}, {
          {filterLabel("Gold Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Glass Bottle"), 1, {2}},
          {filterLabel("Dispenser"), 1, {4}},
          {filterLabel("Microchip (Tier 2)"), 1, {5}},
          {filterLabel("Piston"), 1, {6}},
          {filterLabel("Printed Circuit Board (PCB)"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Microchip (Tier 2)"), 16}}, {
          {filterLabel("Gold Nugget"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Transistor"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Microchip (Tier 1)"), 16}}, {
          {filterLabel("Iron Nugget"), 6, {1, 2, 3, 7, 8, 9}},
          {filterLabel("Redstone"), 2, {4, 6}},
          {filterLabel("Transistor"), 1, {5}}
        }, {10, {}}},
        {{{filterLabel("Transposer"), 16}}, {
          {filterLabel("Iron Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Hopper"), 2, {4, 6}},
          {filterLabel("Bucket"), 1, {5}},
          {filterLabel("Inventory Controller Upgrade"), 1, {2}},
          {filterLabel("Tank Controller Upgrade"), 1, {8}}
        }, {16, {}}},
        {{{filterLabel("Energy Laser Relay"), 16}}, {
          {filterLabel("Obsidian"), 4, {1, 3, 7, 9}},
          {filterLabel("Restonia Crystal"), 2, {4, 6}},
          {filterLabel("Block of Redstone"), 2, {2, 8}},
          {filterLabel("Advanced Coil"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Item Interface"), 16}}, {
          {filterLabel("Basic Coil"), 4, {1, 3, 7, 9}},
          {filterLabel("Restonia Crystal"), 2, {4, 6}},
          {filterLabel("Redstone"), 2, {2, 8}},
          {filterLabel("Chest"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Servo"), 16}}, {
          {filterLabel("Iron Ingot"), 1, {5}},
          {filterLabel("Redstone"), 2, {2, 8}}
        }, {32, {}}},
        {{{filterLabel("Augment: Auxiliary Reception Coil"), 16}}, {
          {filterLabel("Gold Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone Reception Coil"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Augment: Auxiliary Sieve"), 16}}, {
          {filterLabel("Bronze Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone Servo"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Hardened Upgrade Kit"), 16}}, {
          {filterLabel("Invar Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone"), 2, {7, 9}},
          {filterLabel("Bronze Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Reinforced Upgrade Kit"), 16}}, {
          {filterLabel("Electrum Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Fused Quartz"), 2, {7, 9}},
          {filterLabel("Silver Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Signalum Upgrade Kit"), 16}}, {
          {filterLabel("Signalum Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Cryotheum Dust"), 2, {7, 9}},
          {filterLabel("Electrum Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Resonant Upgrade Kit"), 16}}, {
          {filterLabel("Enderium Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Pyrotheum Dust"), 2, {7, 9}},
          {filterLabel("Lumium Gear"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Pulsating Crystal"), 16}}, {
          {filterLabel("Pulsating Iron Nugget"), 8, {1, 2, 3, 4, 6, 7, 8, 9}},
          {filterLabel("Diamond"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Gear"), 16}}, {
          {filterLabel("Redstone Torch"), 4, {2, 4, 6, 8}},
          {filterLabel("Birch Wood Planks"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Modularium Alloy"), 64}}, {
          {filterLabel("Electrical Steel Ingot"), 4, {1, 3, 7, 9}},
          {filterLabel("Platinum Ingot"), 2, {2, 8}},
          {filterLabel("Empowered Palis Crystal"), 2, {4, 6}},
          {filterLabel("Pulsating Crystal"), 1, {5}}
        }, {16, {}}},
        {{{filterLabel("Redstone Engineering Block"), 16}}, {
          {filterLabel("Copper Plate"), 4, {1, 3, 7, 9}},
          {filterLabel("Constantan Ingot"), 4, {2, 4, 6, 8}},
          {filterLabel("Redstone Gear"), 1, {5}}
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
