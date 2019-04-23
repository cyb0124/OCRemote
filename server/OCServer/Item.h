#pragma once
#include <memory>
#include "json.hpp"

struct Item {
  std::string name, label;
  int damage, maxDamage, maxSize;
  bool hasTag;
  nlohmann::json others;

  bool operator==(const Item &other) const;
  bool operator!=(const Item &other) const;
};
using SharedItem = std::shared_ptr<Item>;

struct ItemStack {
  SharedItem item;
  int size;
};
using SharedItemStack = std::shared_ptr<ItemStack>;
SharedItemStack parseItemStack(nlohmann::json&);

namespace ItemFilters {
  struct IndexVisitor {
    virtual ~IndexVisitor() = default;
    virtual void visit(const struct Name&) = 0;
    virtual void visit(const struct Label&) = 0;
    virtual void visit(const struct LabelName&) = 0;
    virtual void visit() = 0;
  };

  struct Base {
    virtual ~Base() = default;
    virtual bool filter(const Item&) = 0;
    virtual void accept(IndexVisitor &v) { v.visit(); }
  };

  struct Name : Base {
    std::string name;
    bool filter(const Item&) override;
    void accept(IndexVisitor &v) override { v.visit(*this); }
  };

  struct Label : Base {
    std::string label;
    bool filter(const Item&) override;
    void accept(IndexVisitor &v) override { v.visit(*this); }
  };

  struct LabelName : Base {
    std::string label, name;
    bool filter(const Item&) override;
    void accept(IndexVisitor &v) override { v.visit(*this); }
  };
}
