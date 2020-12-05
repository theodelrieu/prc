#pragma once

#include <prc/hand.hpp>
#include <prc/parser/ast.hpp>

#include <iosfwd>
#include <string>

namespace prc
{
class hand_range
{
public:
  hand_range() = default;
  hand_range(hand const& from, hand const& to);
  explicit hand_range(parser::ast::hand_range const&);

  hand const& from() const;
  hand const& to() const;

  std::string string() const;

private:
  hand _from;
  hand _to;
};

bool operator==(hand_range const& lhs, hand_range const& rhs);
bool operator!=(hand_range const& lhs, hand_range const& rhs);
bool operator<(hand_range const& lhs, hand_range const& rhs);

std::ostream& operator<<(std::ostream&, hand_range const&);
}
