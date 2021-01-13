#include <prc/parser/api_def.hpp>
#include <prc/parser/config.hpp>

#include <prc/card.hpp>

#include <boost/locale.hpp>

#include <iostream>

namespace prc
{
std::ostream& operator<<(std::ostream& os, prc::rank r)
{
  return os << prc::rank_str[static_cast<int>(r)];
}

std::ostream& operator<<(std::ostream& os, prc::suit s)
{
  return os << prc::suit_str[static_cast<int>(s)];
}

std::ostream& operator<<(std::ostream& os, suitedness s)
{
  return os << prc::suitedness_str[static_cast<int>(s)];
}

namespace parser
{
BOOST_SPIRIT_INSTANTIATE(
    card_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    combo_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    unpaired_hand_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    hand_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    hand_range_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    range_elem_type,
    std::u32string::const_iterator,
    context_type<std::u32string::const_iterator BOOST_PP_COMMA()
                     x3::unicode::space_type>);

BOOST_SPIRIT_INSTANTIATE(
    card_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    combo_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    unpaired_hand_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    hand_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    hand_range_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);
BOOST_SPIRIT_INSTANTIATE(
    range_elem_type,
    std::string::const_iterator,
    context_type<std::string::const_iterator BOOST_PP_COMMA() x3::space_type>);

card_type card()
{
  return _card;
}

combo_type combo()
{
  return _combo;
}

unpaired_hand_type unpaired_hand()
{
  return _unpaired_hand;
}

hand_type hand()
{
  return _hand;
}

hand_range_type hand_range()
{
  return _hand_range;
}

range_elem_type range_elem()
{
  return _range_elem;
}

rank_type const& rank()
{
  static parser::rank_type rank({{U"A", prc::rank::ace},
                                 {U"K", prc::rank::king},
                                 {U"Q", prc::rank::queen},
                                 {U"J", prc::rank::jack},
                                 {U"T", prc::rank::ten},
                                 {U"9", prc::rank::nine},
                                 {U"8", prc::rank::eight},
                                 {U"7", prc::rank::seven},
                                 {U"6", prc::rank::six},
                                 {U"5", prc::rank::five},
                                 {U"4", prc::rank::four},
                                 {U"3", prc::rank::three},
                                 {U"2", prc::rank::two}},
                                "rank");
  return rank;
}

suit_type const& suit()
{
  static parser::suit_type suit({{U"h", prc::suit::heart},
                                 {U"s", prc::suit::spade},
                                 {U"c", prc::suit::club},
                                 {U"d", prc::suit::diamond}},
                                "suit");
  return suit;
}

pair_type const& paired_hand()
{
  static parser::pair_type paired_hand({{U"AA", {prc::rank::ace}},
                                        {U"KK", {prc::rank::king}},
                                        {U"QQ", {prc::rank::queen}},
                                        {U"JJ", {prc::rank::jack}},
                                        {U"TT", {prc::rank::ten}},
                                        {U"99", {prc::rank::nine}},
                                        {U"88", {prc::rank::eight}},
                                        {U"77", {prc::rank::seven}},
                                        {U"66", {prc::rank::six}},
                                        {U"55", {prc::rank::five}},
                                        {U"44", {prc::rank::four}},
                                        {U"33", {prc::rank::three}},
                                        {U"22", {prc::rank::two}}},
                                       "paired_hand");
  return paired_hand;
}

namespace ast
{
bool operator==(card const& lhs, card const& rhs)
{
  return std::tie(lhs.rank, lhs.suit) == std::tie(rhs.rank, rhs.suit);
}

bool operator!=(card const& lhs, card const& rhs)
{
  return !(lhs == rhs);
}

bool operator==(combo const& lhs, combo const& rhs)
{
  return std::tie(lhs.first_card, lhs.second_card) ==
         std::tie(rhs.first_card, rhs.second_card);
}

bool operator!=(combo const& lhs, combo const& rhs)
{
  return !(lhs == rhs);
}

bool operator==(unpaired_hand const& lhs, unpaired_hand const& rhs)
{
  return std::tie(lhs.first_rank, lhs.second_rank, lhs.suitedness) ==
         std::tie(rhs.first_rank, rhs.second_rank, rhs.suitedness);
}

bool operator!=(unpaired_hand const& lhs, unpaired_hand const& rhs)
{
  return !(lhs == rhs);
}

bool operator==(paired_hand const& lhs, paired_hand const& rhs)
{
  return lhs.rank == rhs.rank;
}

bool operator!=(paired_hand const& lhs, paired_hand const& rhs)
{
  return !(lhs == rhs);
}

bool operator==(hand_range const& lhs, hand_range const& rhs)
{
  return std::tie(lhs.from, lhs.to) == std::tie(rhs.from, rhs.to);
}

bool operator!=(hand_range const& lhs, hand_range const& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, card const& c)
{
  return os << c.rank << c.suit;
}

std::ostream& operator<<(std::ostream& os, combo const& c)
{
  return os << c.first_card << c.second_card;
}

std::ostream& operator<<(std::ostream& os, unpaired_hand const& uh)
{
  return os << uh.first_rank << uh.second_rank << uh.suitedness;
}

std::ostream& operator<<(std::ostream& os, paired_hand const& ph)
{
  return os << ph.rank << ph.rank;
}

std::ostream& operator<<(std::ostream& os, hand_range const& hr)
{
  return os << hr.from << '-' << hr.to;
}

inline namespace literals
{

range_elem operator"" _ast_re(char const* str, std::size_t n)
{
  auto s = boost::locale::conv::utf_to_utf<char32_t>(str, str + n);

  x3::error_handler<std::u32string::const_iterator> error_handler(
      s.begin(), s.end(), std::cerr);
  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[x3::expect[parser::range_elem()] > x3::eoi];
  range_elem ret;
  x3::phrase_parse(s.begin(), s.end(), ctx, x3::unicode::space, ret);
  return ret;
}
}
}
}
}
