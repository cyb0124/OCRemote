#include "Serialize.h"
#include "Overload.h"

STable arrayToSTable(std::vector<SValue> &&array) {
  STable result;
  for (size_t i{}; i < array.size(); ++i) {
    if (!array[i].isNull())
      result[static_cast<double>(i)] = std::move(array[i]);
  }
  return result;
}

std::vector<SValue> sTableToArray(STable&& table) {
  std::vector<SValue> result;
  for (auto &entry : table) {
    auto key(std::get<double>(entry.first));
    auto iKey(static_cast<size_t>(key));
    if (iKey != key)
      throw std::runtime_error("non-integer key");
    if (result.size() <= iKey)
      result.resize(iKey + 1);
    result[iKey] = std::move(entry.second);
  }
  return result;
}

namespace {
  struct Serializer {
    std::string operator()(std::monostate) const {
      return "!";
    }

    std::string operator()(double x) const {
      return "#" + std::to_string(x) + "@";
    }

    std::string operator()(const std::string &x) const {
      std::string result{"@"};
      for (char i : x)
        if (i == '@')
          result += "@.";
        else
          result.push_back(i);
      result += "@~";
      return result;
    }

    std::string operator()(bool x) const {
      return x ? "+" : "-";
    }

    std::string operator()(const ValuePtr<STable> &x) const {
      std::string result{"="};
      for (auto &i : *x) {
        result += std::visit(*this, i.first);
        result += i.second.dump();
      }
      return result;
    }
  };
}

std::string SValue::dump() const {
  return std::visit(Serializer{}, *this);
}

std::unique_ptr<Deserializer::State> Deserializer::State::yield() {
  auto s(std::move(env.s));
  env.s = std::move(p);
  return s;
}

void Deserializer::Number::shift(const char *data, size_t size) {
  while (size) {
    char now{*data};
    ++data; --size;
    if ('@' == now) {
      auto s(yield());
      return env.s->reduce(std::stod(buffer), data, size);
    } else {
      buffer.push_back(now);
    }
  }
}

void Deserializer::String::shift(const char *data, size_t size) {
  while (size) {
    char now{*data};
    ++data; --size;
    if (escape) {
      if ('.' == now) {
        buffer.push_back('@');
        escape = false;
      } else if ('~' == now) {
        auto s(yield());
        return env.s->reduce(std::move(buffer), data, size);
      } else {
        throw std::runtime_error("unknown escape");
      }
    } else if ('@' == now) {
      escape = true;
    } else {
      buffer.push_back(now);
    }
  }
}

void Deserializer::Root::shift(const char *data, size_t size) {
  if (!size) return;
  char now{*data};
  ++data; --size;
  auto s(yield());
  switch (now) {
    case '!':
      return env.s->reduce(std::monostate{}, data, size);
    case '#':
      env.enter<Number>();
      return env.s->shift(data, size);
    case '@':
      env.enter<String>();
      return env.s->shift(data, size);
    case '+':
      return env.s->reduce(true, data, size);
    case '-':
      return env.s->reduce(false, data, size);
    case '=':
      env.enter<Table>();
      return env.s->shift(data, size);
    default:
      throw std::runtime_error("invalid tag");
  }
}

void Deserializer::Table::reduce(SValue x, const char *data, size_t size) {
  if (key.has_value()) {
    result.emplace(std::move(*key), std::move(x));
    key.reset();
  } else if (x.isNull()) {
    auto s(yield());
    return env.s->reduce(std::move(result), data, size);
  } else {
    key.emplace(std::move(x));
  }
  env.enter<Root>();
  env.s->shift(data, size);
}

void Deserializer::Table::init() {
  env.enter<Root>();
}

void Deserializer::Start::reduce(SValue x, const char *data, size_t size) {
  env.cb(std::move(x));
  env.enter<Root>();
  env.s->shift(data, size);
}

void Deserializer::Start::init() {
  env.enter<Root>();
}
