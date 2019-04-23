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
  bool Name::filter(const Item &item) {
    return item.name == name;
  }

  bool Label::filter(const Item &item) {
    return item.label == label;
  }

  bool LabelName::filter(const Item &item) {
    return item.label == label && item.name == name;
  }
}
