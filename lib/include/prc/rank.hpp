#pragma once

#include <iosfwd>

namespace prc
{
enum class rank
{
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine,
  ten,
  jack,
  queen,
  king,
  ace,
};

inline constexpr char rank_str[13] = {
    '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'};

std::ostream& operator<<(std::ostream&, rank);
}
