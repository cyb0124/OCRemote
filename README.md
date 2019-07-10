# OCRemote
OCRemote is a OpenComputers program by cyb0124 for item management and auto-crafting. Main features include:
  - Extensively reuse of same machines for multiple recipes.
  - Prioritization of recipes based on the current number of items stored. (e.g. deciding which crop to grow.)
  - Robust handling of multiple-input recipes to prevent clogging (e.g. alloy furnaces).
  - Preventing recipes from using up the last seed/sapling/etc.

## Server and clients
OCRemote includes a TCP server program written in C++ that needs be run outside the minecraft world. In fact, actual computations all happen in the server. The computers in minecraft world merely execute the commands sent by the server. Multiple clients can connect to the same server, which allows parallelization of inventory manipulation operations.

## Bus
OCRemote requires a shared inventory to move items around. This inventory is called as the "bus" in the source code. The bus can be implemented using EnderStorage's ender chests, or using ActuallyAdditions' item lasers.\
![Viewing inside the bus inventory](busDemo2.gif "Viewing inside the bus inventory")

## Storage
OCRemote currently supports 3 different types of storages:
  - **Chests**\
    OCRemote will use chests the most efficient way, i.e. coalesce item stacks to avoid wasting slots.
  - **StorageDrawers** or equivalent
  - **ME system**\
    OpenComputers' access to ME system is slow, so OCRemote uses multiple computer to access the same ME system to parallelize accesses.

## Auto Crafting
OCRemote currently supports the following types of auto-crafting processes:
  - **ProcessSlotted**\
    This process is intended for machines that can only run 1 recipe at once and the input items need to go into specific slot in the correct ratio. OCRemote will only execute recipes that input items match the items already in the machine.
  - **ProcessCraftingRobot**\
    This process uses a single crafting robot to handle all grid crafting recipes. It also allows non-consumable items in recipes (e.g. Pam's Harvestcraft recipes that require a utensil).
  - **ProcessRFToolsControlWorkbench**\
    Same as ProcessCraftingRobot, but uses RFTools Control's Workbench as the crafter.\
    ![Grid crafting with workbench](workbench.gif "Grid crafting with workbench")
  - **ProcessBuffered**\
    This process is intended for machines that can run multiple recipes at once, or for general buffering/pipelining of recipe inputs. In additional to recipes, it also allow items to be constantly refilled at the target inventory. It allows limiting each individual recipe's maximum number of item being processed. It also respects the ratio of the input items, which is useful for machines such as ExCompressum's Auto Compressor.
  - **ProcessScatteringWorkingSet**\
    This process is intended for machine that can run multiple recipes at once but independently for each slot. This process will try to spread out input items among slots to help with parallelization.
  - **ProcessInputless** and **ProcessHeterogeneousInputless**\
    These processes are for machines that passively generate outputs (e.g. cobblestone generators).
  - **ProcessReactorHysteresis**\
    This process is a simple hysteresis feedback controller for big/extreme reactors.
  - **ProcessReactorProportional**\
    This process is a simple proportional feedback controller for big/extreme reactors.
