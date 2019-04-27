#include <boost/container_hash/hash.hpp>
#include "Item.h"

bool Item::operator==(const Item &other) const {
  return name == other.name
      && label == other.label
      && damage == other.damage
      && maxDamage == other.maxDamage
      && maxSize == other.maxSize
      && hasTag == other.hasTag
      && others == other.others;
}

bool Item::operator!=(const Item &other) const {
  return !(*this == other);
}

size_t Item::hash() const {
  size_t hash = 0;
  boost::hash_combine(hash, name);
  boost::hash_combine(hash, label);
  boost::hash_combine(hash, damage);
  boost::hash_combine(hash, maxDamage);
  boost::hash_combine(hash, maxSize);
  boost::hash_combine(hash, hasTag);
  boost::hash_combine(hash, std::hash<nlohmann::json>()(others));
  return hash;
}

SharedItemStack parseItemStack(nlohmann::json &j) {
  SharedItemStack result(std::make_shared<ItemStack>());
  result->item = std::make_shared<Item>();

  auto getToAndErase([&j](const std::string &key, auto &to) {
    j.at(key).get_to(to);
    j.erase(key);
  });
  getToAndErase("size", result->size);
  getToAndErase("name", result->item->name);
  getToAndErase("label", result->item->label);
  getToAndErase("damage", result->item->damage);
  getToAndErase("maxDamage", result->item->maxDamage);
  getToAndErase("maxSize", result->item->maxSize);
  getToAndErase("hasTag", result->item->hasTag);
  result->item->others = std::move(j);

  return result;
}

std::vector<SharedItemStack> cloneInventorySizes(const std::vector<SharedItemStack> &inventory) {
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

int insertIntoInventory(std::vector<SharedItemStack> &inventory, const SharedItem &item, int size) {
  size = std::min(size, item->maxSize);
  int result = 0;
  SharedItemStack *firstEmptySlot = nullptr;
  for (auto &i : inventory) {
    if (!i) {
      if (!firstEmptySlot)
        firstEmptySlot = &i;
    } else if (*i->item == *item) {
      int toProc = std::min(size, i->item->maxSize - i->size);
      i->size += toProc;
      result += toProc;
      size -= toProc;
    }
  }
  if (size && firstEmptySlot) {
    auto &stack = *(*firstEmptySlot = std::make_shared<ItemStack>());
    stack.item = item;
    stack.size = size;
    return result + size;
  } else {
    return result;
  }
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
