#include <boost/container_hash/hash.hpp>
#include "Item.h"

SharedItem placeholderItem(std::make_shared<Item>(Item{
  "OCRemote.placeholderItem.name",
  "OCRemote.placeholderItem.label",
  0, 0, 1, false, {}
}));

void Item::serialize(SValue &s) const {
  auto &t(std::get<STable>(s = others));
  t["name"] = name;
  t["label"] = label;
  t["damage"] = static_cast<double>(damage);
  t["maxDamage"] = static_cast<double>(maxDamage);
  t["maxSize"] = static_cast<double>(maxSize);
  t["hasTag"] = hasTag;
}

bool operator==(const Item &x, const Item &y) {
  if (&x == &y)
    return true;
  return x.name == y.name
      && x.label == y.label
      && x.damage == y.damage
      && x.maxDamage == y.maxDamage
      && x.maxSize == y.maxSize
      && x.hasTag == y.hasTag
      && x.others == y.others;
}

size_t hash_value(const Item &x) {
  size_t result{};
  boost::hash_combine(result, x.name);
  boost::hash_combine(result, x.label);
  boost::hash_combine(result, x.damage);
  boost::hash_combine(result, x.maxDamage);
  boost::hash_combine(result, x.maxSize);
  boost::hash_combine(result, x.hasTag);
  boost::hash_combine(result, serialize(x.others));
  return result;
}

namespace {
  template<typename T> struct TransferHelper { void operator()(SValue &from, T &to) const { to = std::move(std::get<T>(from)); } };
  template<> struct TransferHelper<int> { void operator()(SValue &from, int &to) const { to = static_cast<int>(std::get<double>(from)); } };
  template<typename T> void transferHelper(SValue &from, T &to) { TransferHelper<T>()(from, to); }
}

SharedItemStack parseItemStack(SValue &&s) {
  auto result(std::make_shared<ItemStack>());
  result->item = std::make_shared<Item>();
  auto &t(std::get<STable>(s));
  auto transfer([&t](const std::string &key, auto &to) {
    auto itr(t.find(key));
    if (itr == t.end())
      throw std::runtime_error("key \"" + key + "\" not found");
    transferHelper(itr->second, to);
    t.erase(itr);
  });
  struct Int { int &to; void operator()(SValue &x) const { to = static_cast<int>(std::get<double>(x)); } };
  transfer("size", result->size);
  transfer("name", result->item->name);
  transfer("label", result->item->label);
  transfer("damage", result->item->damage);
  transfer("maxDamage", result->item->maxDamage);
  transfer("maxSize", result->item->maxSize);
  transfer("hasTag", result->item->hasTag);
  result->item->others = std::move(t);
  return result;
}

std::vector<SharedItemStack> cloneInventory(const std::vector<SharedItemStack> &inventory) {
  std::vector<SharedItemStack> result;
  result.reserve(inventory.size());
  for (auto &i : inventory) {
    if (i)
      result.emplace_back(std::make_shared<ItemStack>(*i));
    else
      result.emplace_back();
  }
  return result;
}

InsertResult insertIntoInventory(std::vector<SharedItemStack> &inventory, const SharedItem &item, int size) {
  InsertResult result;
  size = std::min(size, item->maxSize);
  std::optional<size_t> firstEmptySlot;
  for (size_t slot{}; size && slot < inventory.size(); ++slot) {
    auto &stack(inventory[slot]);
    if (!stack) {
      if (!firstEmptySlot.has_value())
        firstEmptySlot.emplace(slot);
    } else if (*stack->item == *item) {
      int toProc{std::min(size, item->maxSize - stack->size)};
      stack->size += toProc;
      result.totalSize += toProc;
      result.actions.emplace_back(slot, toProc);
      size -= toProc;
    }
  }
  if (size && firstEmptySlot.has_value()) {
    auto &stack{*(inventory[*firstEmptySlot] = std::make_shared<ItemStack>())};
    stack.item = item;
    stack.size = size;
    result.totalSize += size;
    result.actions.emplace_back(*firstEmptySlot, size);
  }
  return result;
}

namespace ItemFilters {
  bool Name::filter(const Item &item) const {
    return item.name == name;
  }

  bool Label::filter(const Item &item) const {
    return item.label == label;
  }

  bool LabelName::filter(const Item &item) const {
    return item.label == label && item.name == name;
  }
}
