#pragma once
#include "Factory.h"

class ProcessHeterogeneous : public Process {
  std::string name;
  XNetCoord pos;
  int side;
  int maxInProc;
  std::vector<Recipe<std::monostate>> recipes;
public:
  ProcessHeterogeneous(std::string name, const XNetCoord &pos, int side, int maxInProc, std::vector<Recipe<std::monostate>> recipes)
    :name(std::move(name)), pos(pos), side(side), maxInProc(maxInProc), recipes(std::move(recipes)) {}
  virtual void filterInputSlots(std::vector<SharedItemStack> &inventory) {}
  SharedPromise<std::monostate> cycle(Factory&) override;
};
