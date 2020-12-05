#pragma once

#include <prc/rank.hpp>
#include <prc/suit.hpp>
#include <prc/suitedness.hpp>

#include <iosfwd>

#include <boost/fusion/include/io.hpp>
#include <boost/variant.hpp>

namespace prc::parser::ast
{
struct card
{
  prc::rank rank;
  prc::suit suit;
};

struct combo
{
  ast::card first_card;
  ast::card second_card;
};

struct unpaired_hand
{
  prc::rank first_rank;
  prc::rank second_rank;
  prc::suitedness suitedness;
};

struct paired_hand
{
  prc::rank rank;
};

using hand = boost::variant<paired_hand, unpaired_hand>;

struct hand_range
{
  hand from;
  hand to;
};

using range_elem = boost::variant<combo, hand_range, hand>;

bool operator==(card const& lhs, card const& rhs);
bool operator!=(card const& lhs, card const& rhs);

bool operator==(combo const& lhs, combo const& rhs);
bool operator!=(combo const& lhs, combo const& rhs);

bool operator==(unpaired_hand const& lhs, unpaired_hand const& rhs);
bool operator!=(unpaired_hand const& lhs, unpaired_hand const& rhs);

bool operator==(paired_hand const& lhs, paired_hand const& rhs);
bool operator!=(paired_hand const& lhs, paired_hand const& rhs);

bool operator==(hand_range const& lhs, hand_range const& rhs);
bool operator!=(hand_range const& lhs, hand_range const& rhs);

using boost::fusion::operator<<;

std::ostream& operator<<(std::ostream&, card const&);
std::ostream& operator<<(std::ostream&, combo const&);
std::ostream& operator<<(std::ostream&, unpaired_hand const&);
std::ostream& operator<<(std::ostream&, paired_hand const&);
std::ostream& operator<<(std::ostream&, hand_range const&);

inline namespace literals
{
range_elem operator"" _ast_re(char const*, std::size_t);
}
}
