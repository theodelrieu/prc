#pragma once

#include <prc/paired_hand.hpp>
#include <prc/parser/ast.hpp>
#include <prc/unpaired_hand.hpp>

#include <boost/variant2/variant.hpp>

#include <iosfwd>
#include <optional>
#include <string>

namespace prc
{
class hand
{
public:
  hand() = default;
  explicit hand(unpaired_hand const&);
  explicit hand(paired_hand);
  explicit hand(parser::ast::hand const&);

  template <typename T>
  bool holds_alternative() const;

  template <typename T>
  std::optional<T> get_if() const;

  int index() const;

  template <typename T>
  T const& get() const;

  template <typename Callable>
  decltype(auto) visit(Callable&&) const;

  std::string string() const;

  friend bool operator==(hand const& lhs, hand const& rhs);
  friend bool operator<(hand const& lhs, hand const& rhs);

private:
  boost::variant2::variant<paired_hand, unpaired_hand> _v;
};

bool operator!=(hand const& lhs, hand const& rhs);
std::ostream& operator<<(std::ostream&, hand const&);

template <typename T>
bool hand::holds_alternative() const
{
  return boost::variant2::holds_alternative<T>(_v);
}

template <typename T>
std::optional<T> hand::get_if() const
{
  auto p = boost::variant2::get_if<T>(&_v);
  if (!p)
    return std::nullopt;
  return *p;
}

template <typename T>
T const& hand::get() const
{
  return boost::variant2::get<T>(_v);
}

template <typename Callable>
decltype(auto) hand::visit(Callable&& f) const
{
  return boost::variant2::visit(std::forward<Callable>(f), _v);
}
}
