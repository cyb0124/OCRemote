#include "Processes.h"

SharedPromise<std::monostate> ProcessHeterogeneous::cycle(Factory &factory) {
  if (factory.getDemand(recipes).empty())
    return makeEmptyPromise(factory.s.io);
  auto action(std::make_shared<Actions::ListXN>());
  action->inv = factory.baseInv;
  action->pos = pos - factory.basePos;
  action->side = side;
  factory.s.enqueueAction(factory.baseClient, action);
  return action->then([this, wk(weak_from_this()), &factory, &io(factory.s.io)](std::vector<SharedItemStack> inventory) {
    if (wk.expired())
      return makeEmptyPromise(io);
    filterInputSlots(inventory);
    int remainingMaxInProc = maxInProc;
    for (auto &item : inventory)
      if (item)
        remainingMaxInProc -= item->size;
    std::vector<SharedPromise<std::monostate>> promises;
    bool hasNew = remainingMaxInProc > 0;
    while (hasNew) {
      hasNew = false;
      auto demands(factory.getDemand(recipes));
      for (auto &demand : demands) {
        int itemsInEachSet = 0;
        for (auto &item : demand.recipe->in)
          itemsInEachSet += item.size;
        int toProc = std::min(demand.inAvail, remainingMaxInProc / itemsInEachSet);
        while (toProc) {
          auto newInventory(cloneInventorySizes(inventory));
          bool success = true;
          for (size_t i = 0; i < demand.in.size(); ++i) {
            int toInsert = toProc * demand.recipe->in[i].size;
            if (toInsert != insertIntoInventory(newInventory, demand.in[i], toInsert)) {
              success = false;
              break;
            }
          }
          if (success) {
            inventory = std::move(newInventory);
            break;
          }
          --toProc;
        }
        if (toProc) {
          hasNew = true;
          remainingMaxInProc -= toProc * itemsInEachSet;
          for (size_t i = 0; i < demand.in.size(); ++i)
            factory.extract(promises, name, demand.in[i], toProc * demand.recipe->in[i].size, pos, side);
          break;
        }
      }
    }
    if (promises.empty())
      return makeEmptyPromise(io);
    return Promise<std::monostate>::all(promises)->map([](auto) -> std::monostate { return {}; });
  });
}
