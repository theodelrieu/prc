#include <prc/paired_hand.hpp>

#include <ostream>

namespace prc
{
paired_hand::paired_hand(prc::rank r) : _r(r)
{
}

paired_hand::paired_hand(parser::ast::paired_hand ph) : _r(ph.rank)
{
}

prc::rank paired_hand::rank() const
{
  return _r;
}

std::string paired_hand::string() const
{
  return std::string(2, rank_str[static_cast<int>(_r)]);
}

bool operator==(paired_hand const& lhs, paired_hand const& rhs) noexcept
{
  return lhs.rank() == rhs.rank();
}

bool operator!=(paired_hand const& lhs, paired_hand const& rhs) noexcept
{
  return !(lhs == rhs);
}

bool operator<(paired_hand const& lhs, paired_hand const& rhs) noexcept
{
  return lhs.rank() < rhs.rank();
}

std::ostream& operator<<(std::ostream& os, paired_hand const& ph)
{
  return os << ph.string();
}
}
