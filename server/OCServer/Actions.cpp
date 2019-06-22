#include "Actions.h"

namespace Actions {
  void Print::dump(STable &s) {
    s["op"] = "print";
    s["color"] = static_cast<double>(color);
    s["text"] = std::move(text);
    if (beep > 0) s["beep"] = beep;
  }

  void List::dump(STable &s) {
    s["op"] = "list";
    s["side"] = static_cast<double>(side);
    s["inv"] = std::move(inv);
  }

  void List::onResult(SValue s) {
    auto sItems(sTableToArray(std::move(std::get<STable>(s))));
    std::vector<SharedItemStack> items;
    items.reserve(sItems.size());
    for (size_t i{}; i < sItems.size(); ++i) {
      if (std::holds_alternative<std::string>(sItems[i]))
        items.emplace_back();
      else
        items.emplace_back(parseItemStack(std::move(sItems[i])));
    }
    Promise<std::vector<SharedItemStack>>::onResult(std::move(items));
  }

  void ListXN::dump(STable &s) {
    List::dump(s);
    s["op"] = "listXN";
    s["x"] = static_cast<double>(pos.x);
    s["y"] = static_cast<double>(pos.y);
    s["z"] = static_cast<double>(pos.z);
  }

  void ListME::dump(STable &s) {
    s["op"] = "listME";
    s["inv"] = std::move(inv);
  }

  void ListME::onResult(SValue s) {
    auto sItems(sTableToArray(std::move(std::get<STable>(s))));
    std::vector<SharedItemStack> items;
    items.reserve(sItems.size());
    for (size_t i{}; i < sItems.size(); ++i)
      items.emplace_back(parseItemStack(std::move(sItems[i])));
    Promise<std::vector<SharedItemStack>>::onResult(std::move(items));
  }

  void XferME::dump(STable &t) {
    t["op"] = "xferME";
    t["size"] = static_cast<double>(size);
    t["filter"] = std::move(filter);
    t["args"] = arrayToSTable(std::move(args));
    t["inv"] = std::move(inv);
    t["me"] = std::move(me);
  }

  void Call::dump(STable &s) {
    s["op"] = "call";
    s["fn"] = std::move(fn);
    s["args"] = arrayToSTable(std::move(args));
    s["inv"] = std::move(inv);
  }

  void Call::onResult(SValue s) {
    Promise<SValue>::onResult(std::move(s));
  }
}
