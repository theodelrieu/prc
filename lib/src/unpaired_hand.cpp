#include <prc/unpaired_hand.hpp>

#include <sstream>
#include <stdexcept>

namespace prc
{
unpaired_hand::unpaired_hand(rank r1, rank r2, suitedness suited)
  : _high(r1), _low(r2), _suitedness(suited)
{
  if (r1 == r2)
    throw std::runtime_error{"unpaired_hand cannot have the same rank twice"};
  if (r1 < r2)
    std::swap(_high, _low);
}

unpaired_hand::unpaired_hand(parser::ast::unpaired_hand const& uh)
  : unpaired_hand(uh.first_rank, uh.second_rank, uh.suitedness)
{
}

rank unpaired_hand::high() const
{
  return _high;
}

rank unpaired_hand::low() const
{
  return _low;
}

bool unpaired_hand::suited() const
{
  return _suitedness == suitedness::suited;
}

bool unpaired_hand::offsuit() const
{
  return !suited();
}

std::string unpaired_hand::string() const
{
  std::stringstream ss;
  ss << _high << _low << _suitedness;
  return ss.str();
}

bool operator==(unpaired_hand const& lhs, unpaired_hand const& rhs) noexcept
{
  return lhs.high() == rhs.high() && lhs.low() == rhs.low() &&
         lhs.suited() == rhs.suited();
}

bool operator!=(unpaired_hand const& lhs, unpaired_hand const& rhs) noexcept
{
  return !(lhs == rhs);
}

bool operator<(unpaired_hand const& lhs, unpaired_hand const& rhs) noexcept
{
  return std::tuple{lhs.high(), lhs.low(), lhs.suited()} <
         std::tuple{rhs.high(), rhs.low(), rhs.suited()};
}

std::ostream& operator<<(std::ostream& os, unpaired_hand const& uh)
{
  return os << uh.string();
}
}
