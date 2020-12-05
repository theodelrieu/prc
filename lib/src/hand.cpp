#include <prc/hand.hpp>

#include <sstream>

namespace prc
{
hand::hand(unpaired_hand const& uh) : _v(uh)
{
}

hand::hand(paired_hand ph) : _v(ph)
{
}

hand::hand(parser::ast::hand const& h)
{
  if (auto p = boost::get<parser::ast::unpaired_hand>(&h))
    _v = unpaired_hand{*p};
  else
    _v = paired_hand{boost::get<parser::ast::paired_hand>(h)};
}

std::string hand::string() const
{
  return boost::variant2::visit([](auto& e) { return e.string(); }, _v);
}

int hand::index() const
{
  return _v.index();
}

bool operator==(hand const& lhs, hand const& rhs)
{
  return lhs._v == rhs._v;
}

bool operator!=(hand const& lhs, hand const& rhs)
{
  return !(lhs == rhs);
}

bool operator<(hand const& lhs, hand const& rhs)
{
  return lhs._v < rhs._v;
}

std::ostream& operator<<(std::ostream& os, hand const& h)
{
  return os << h.string();
}
}
