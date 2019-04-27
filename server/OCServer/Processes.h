#pragma once
#include "Factory.h"

class ProcessSingleBlock : public Process {
  std::string name;
  XNetCoord pos;
  int side;
  int maxInProc;
  bool homogeneous;
  std::vector<Recipe<int>> recipes;
public:
  ProcessSingleBlock(std::string name, const XNetCoord &pos, int side, int maxInProc, bool homogeneous, std::vector<Recipe<int>> recipes)
    :name(std::move(name)), pos(pos), side(side), maxInProc(maxInProc), homogeneous(homogeneous), recipes(std::move(recipes)) {}
  virtual void filterInputSlots(std::vector<SharedItemStack> &inventory) {}
  SharedPromise<std::monostate> cycle(Factory&) override;
};
