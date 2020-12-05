#pragma once

#include <prc/combo.hpp>
#include <prc/hand.hpp>
#include <prc/hand_range.hpp>
#include <prc/parser/ast.hpp>

#include <boost/variant2/variant.hpp>

#include <iosfwd>
#include <optional>
#include <string>

namespace prc
{
class range_elem
{
public:
  range_elem() = default;
  explicit range_elem(hand const&);
  explicit range_elem(combo const&);
  explicit range_elem(hand_range const&);
  explicit range_elem(parser::ast::range_elem const&);

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

  friend bool operator==(range_elem const& lhs, range_elem const& rhs);
  friend bool operator<(range_elem const& lhs, range_elem const& rhs);

private:
  boost::variant2::variant<combo, hand, hand_range> _v;
};

bool operator!=(range_elem const& lhs, range_elem const& rhs);
std::ostream& operator<<(std::ostream&, range_elem const&);

template <typename T>
bool range_elem::holds_alternative() const
{
  return boost::variant2::holds_alternative<T>(_v);
}

template <typename T>
std::optional<T> range_elem::get_if() const
{
  auto p = boost::variant2::get_if<T>(&_v);
  if (!p)
    return std::nullopt;
  return *p;
}

template <typename T>
T const& range_elem::get() const
{
  return boost::variant2::get<T>(_v);
}

template <typename Callable>
decltype(auto) range_elem::visit(Callable&& f) const
{
  return boost::variant2::visit(std::forward<Callable>(f), _v);
}

inline namespace literals
{
range_elem operator"" _re(char const*, std::size_t);
}
}
