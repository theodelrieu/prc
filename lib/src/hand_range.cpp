#include <prc/hand_range.hpp>

#include <sstream>
#include <stdexcept>
#include <tuple>

namespace prc
{
hand_range::hand_range(hand const& from, hand const& to) : _from(from), _to(to)
{
  if (_from.index() != _to.index())
    throw std::runtime_error{"from and to must have the same index"};
  if (_to < _from)
    std::swap(_from, _to);
}

hand_range::hand_range(parser::ast::hand_range const& hr)
  : hand_range(hand{hr.from}, hand{hr.to})
{
}

hand const& hand_range::from() const
{
  return _from;
}

hand const& hand_range::to() const
{
  return _to;
}

std::string hand_range::string() const
{
  // all softwares do not support XX+
  std::stringstream ss;
  ss << _from << '-' << _to;
  return ss.str();
}

bool operator==(hand_range const& lhs, hand_range const& rhs)
{
  return std::tie(lhs.from(), lhs.to()) == std::tie(rhs.from(), rhs.to());
}

bool operator!=(hand_range const& lhs, hand_range const& rhs)
{
  return !(lhs == rhs);
}

bool operator<(hand_range const& lhs, hand_range const& rhs)
{
  return std::tie(lhs.from(), lhs.to()) < std::tie(rhs.from(), rhs.to());
}

std::ostream& operator<<(std::ostream& os, hand_range const& hr)
{
  return os << hr.string();
}
}
