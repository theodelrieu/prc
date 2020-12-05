#include <prc/card.hpp>
#include <prc/parser/api.hpp>

#include <tuple>

namespace prc
{
card::card(prc::rank r, prc::suit s) : _r(r), _s(s)
{
}

card::card(parser::ast::card c) : card(c.rank, c.suit)
{
}

prc::rank const& card::rank() const
{
  return _r;
}

prc::suit const& card::suit() const
{
  return _s;
}

std::string card::string() const
{
  std::string res;
  res += rank_str[static_cast<int>(_r)];
  res += suit_str[static_cast<int>(_s)];
  return res;
}

bool operator==(card const& lhs, card const& rhs) noexcept
{
  return std::tie(lhs.rank(), lhs.suit()) == std::tie(rhs.rank(), rhs.suit());
}

bool operator!=(card const& lhs, card const& rhs) noexcept
{
  return !(lhs == rhs);
}

bool operator<(card const& lhs, card const& rhs) noexcept
{
  return std::tie(lhs.rank(), lhs.suit()) < std::tie(rhs.rank(), rhs.suit());
}

std::ostream& operator<<(std::ostream& os, card const& c)
{
  return os << c.string();
}
}
