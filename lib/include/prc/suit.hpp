#pragma once

#include <iosfwd>

namespace prc
{
enum class suit
{
  club,
  diamond,
  heart,
  spade,
};

inline constexpr char suit_str[4] = {'c', 'd', 'h', 's'};

std::ostream& operator<<(std::ostream&, suit);
}
