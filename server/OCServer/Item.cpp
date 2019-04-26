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
