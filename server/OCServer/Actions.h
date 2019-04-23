#pragma once
#include <vector>
#include <variant>
#include <optional>
#include <functional>
#include <boost/noncopyable.hpp>
#include "json.hpp"
#include "Item.h"

template<typename T>
using FailableCb = std::function<void(std::optional<T>)>;

namespace Actions {
  enum {
    bottom = 0, down  = 0, yn = 0,
    top    = 1, up    = 1, yp = 1,
    back   = 2, north = 2, zn = 2,
    front  = 3, south = 3, zp = 3,
    right  = 4, west  = 4, xn = 4,
    left   = 5, east  = 5, xp = 5
  };

  struct Base : private boost::noncopyable {
    virtual ~Base() = default;
    virtual void dump(nlohmann::json&) = 0;
    virtual void onResponse(nlohmann::json&) = 0;
    virtual void onError() = 0;
  };

  template<typename T>
  struct Impl : Base {
    FailableCb<T> cb;
    void onError() override {
      if (cb) cb(std::nullopt);
    }
  };

  struct ImplNoResult : Impl<std::monostate> {
    void onResponse(nlohmann::json&) override;
  };

  struct Print : ImplNoResult {
    std::string text;
    uint32_t color = 0xffffff;
    std::optional<float> beep;
    void dump(nlohmann::json&) override;
  };

  struct List : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    int side;
    void dump(nlohmann::json&) override;
    void onResponse(nlohmann::json&) override;
  };

  struct ListXN : List {
    int x, y, z;
    void dump(nlohmann::json&) override;
  };

  struct ListME : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    void dump(nlohmann::json&) override;
    void onResponse(nlohmann::json&) override;
  };

  struct XferME : ImplNoResult {
    std::string me, inv;
    nlohmann::json filter;
    int fromSide, toSide, slot;
    void dump(nlohmann::json&) override;
  };

  struct Call : Impl<nlohmann::json> {
    std::string inv, fn;
    nlohmann::json args = nlohmann::json::array();
    void dump(nlohmann::json&) override;
    void onResponse(nlohmann::json&) override;
  };
}

using SharedAction = std::shared_ptr<Actions::Base>;
