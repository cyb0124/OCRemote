#ifndef _ITEM_H_
#define _ITEM_H_
#include "Serialize.h"

struct Item {
  std::string name, label;
  int damage, maxDamage, maxSize;
  bool hasTag;
  STable others;
  void serialize(SValue &s) const;
};
bool operator==(const Item &x, const Item &y);
inline bool operator!=(const Item &x, const Item &y) { return !(x == y); }
size_t hash_value(const Item&);
using SharedItem = std::shared_ptr<Item>;
struct SharedItemHash { size_t operator()(const SharedItem &x) const { return hash_value(*x); } };
struct SharedItemEqual { bool operator()(const SharedItem &x, const SharedItem &y) const { return *x == *y; } };
extern SharedItem placeholderItem;

struct ItemStack {
  SharedItem item;
  int size;
};
using SharedItemStack = std::shared_ptr<ItemStack>;
SharedItemStack parseItemStack(SValue&&);
std::vector<SharedItemStack> cloneInventory(const std::vector<SharedItemStack> &inventory);

struct InsertResult {
  int totalSize{};
  std::vector<std::pair<size_t, int>> actions;
};
InsertResult insertIntoInventory(std::vector<SharedItemStack> &inventory, const SharedItem &item, int size);

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
    const std::string name;
    explicit Name(std::string name) :name(std::move(name)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };

  struct Label final : Base {
    const std::string label;
    explicit Label(std::string label) :label(std::move(label)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };

  struct LabelName final : Base {
    const std::string label, name;
    explicit LabelName(std::string label, std::string name)
      :label(std::move(label)), name(std::move(name)) {}
    bool filter(const Item&) const override;
    void accept(IndexVisitor &v) const override { v.visit(*this); }
  };
}
using SharedItemFilter = std::shared_ptr<ItemFilters::Base>;

#define DEF_FILTER_SHORTCUT(type)\
  template<typename ...Ts>\
  inline std::shared_ptr<ItemFilters::type> filter##type(Ts &&...xs) {\
    return std::make_shared<ItemFilters::type>(std::forward<Ts>(xs)...);\
  }
DEF_FILTER_SHORTCUT(Name)
DEF_FILTER_SHORTCUT(Label)
DEF_FILTER_SHORTCUT(LabelName)

template<typename F> SharedItemFilter inline filterFn(F fn) {
  struct Impl : ItemFilters::Base {
    F fn;
    Impl(F fn) :fn(std::move(fn)) {}
    virtual bool filter(const Item &x) const { return fn(x); }
  };
  return std::make_shared<Impl>(std::move(fn));
}

#endif
