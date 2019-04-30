#include "Actions.h"

namespace Actions {
  void Print::dump(nlohmann::json &j) {
    j = {{"op", "print"}, {"color", color}};
    j["text"] = std::move(text);
    if (beep > 0) j["beep"] = beep;
  }

  void List::dump(nlohmann::json &j) {
    j = {{"op", "list"}, {"side", side}};
    j["inv"] = std::move(inv);
  }

  void List::onResult(nlohmann::json j) {
    std::vector<SharedItemStack> items(j.size());
    for (size_t i = 0; i < items.size(); ++i) {
      auto &ji = j.at(i);
      if (!ji.is_string())
        items[i] = parseItemStack(ji);
    }
    Promise<std::vector<SharedItemStack>>::onResult(std::move(items));
  }

  void ListXN::dump(nlohmann::json &j) {
    List::dump(j);
    j["op"] = "listXN";
    j["x"] = pos.x;
    j["y"] = pos.y;
    j["z"] = pos.z;
  }

  void ListME::dump(nlohmann::json &j) {
    j = {{"op", "listME"}};
    j["inv"] = std::move(inv);
  }

  void ListME::onResult(nlohmann::json j) {
    std::vector<SharedItemStack> items(j.size());
    for (size_t i = 0; i < items.size(); ++i)
      items[i] = parseItemStack(j.at(i));
    Promise<std::vector<SharedItemStack>>::onResult(std::move(items));
  }

  void XferME::dump(nlohmann::json &j) {
    j = {{"op", "xferME"}, {"size", size}};
    j["filter"] = std::move(filter);
    j["args"] = std::move(args);
    j["inv"] = std::move(inv);
    j["me"] = std::move(me);
  }

  void Call::dump(nlohmann::json &j) {
    j = {{"op", "call"}};
    j["fn"] = std::move(fn);
    j["args"] = std::move(args);
    j["inv"] = std::move(inv);
  }

  void Call::onResult(nlohmann::json j) {
    Promise<nlohmann::json>::onResult(std::move(j));
  }
}
