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
  size_t hash() const;
};
using SharedItem = std::shared_ptr<Item>;

struct SharedItemHash {
  size_t operator()(const SharedItem &x) const {
    return x->hash();
  }
};

struct SharedItemEqual {
  bool operator()(const SharedItem &x, const SharedItem &y) const {
    return *x == *y;
  }
};

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
    virtual void visit(const struct Base&) = 0;
  };

  struct Base {
    virtual ~Base() = default;
    virtual bool filter(const Item&) const = 0;
    virtual void accept(IndexVisitor &v) const { v.visit(*this); }
  };

  struct Name final : Base {
    std::string name;
    explicit Name(std::string name) :name(std::move(name)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };

  struct Label final : Base {
    std::string label;
    explicit Label(std::string label) :label(std::move(label)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };

  struct LabelName final : Base {
    std::string label, name;
    explicit LabelName(std::string label, std::string name)
      :label(std::move(label)), name(std::move(name)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };
}

using SharedItemFilter = std::shared_ptr<ItemFilters::Base>;
