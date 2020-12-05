#pragma once

#include <iosfwd>

namespace prc
{
enum class suitedness
{
  offsuit,
  suited,
};

inline constexpr char suitedness_str[2] = {'o', 's'};

std::ostream& operator<<(std::ostream&, suitedness);
}
