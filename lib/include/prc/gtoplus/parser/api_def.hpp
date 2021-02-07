#pragma once

// #define BOOST_SPIRIT_X3_DEBUG
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/tuple/tuple_tie.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include <boost/variant.hpp>

#include <prc/detail/unicode.hpp>
#include <prc/gtoplus/parser/api.hpp>
#include <prc/gtoplus/parser/ast.hpp>
#include <prc/gtoplus/parser/ast_adapted.hpp>
#include <prc/gtoplus/parser/config.hpp>
#include <prc/parser/api.hpp>
#include <prc/parser/as_type.hpp>
#include <prc/parser/ast.hpp>

#include <cassert>
#include <cstring>

namespace prc::gtoplus::parser
{
struct error_handler
{
  template <typename Iterator, typename Exception, typename Context>
  x3::error_handler_result on_error(Iterator& first,
                                    Iterator const& last,
                                    Exception const& x,
                                    Context const& context)
  {
    using namespace std::string_literals;

    auto& error_handler = x3::get<x3::error_handler_tag>(context);
    auto msg = "Expecting "s + x.which() + " here:";
    error_handler(x.where(), msg);
    return x3::error_handler_result::fail;
  }
};

auto const& sorted_hands()
{
  using namespace prc::parser::ast::literals;
  // clang-format off
  static std::vector hands = []{
    std::vector elems{
    "AA"_ast_re, "AKs"_ast_re, "AQs"_ast_re, "AJs"_ast_re, "ATs"_ast_re, "A9s"_ast_re, "A8s"_ast_re, "A7s"_ast_re, "A6s"_ast_re, "A5s"_ast_re, "A4s"_ast_re, "A3s"_ast_re, "A2s"_ast_re,
    "AKo"_ast_re, "KK"_ast_re, "KQs"_ast_re, "KJs"_ast_re, "KTs"_ast_re, "K9s"_ast_re, "K8s"_ast_re, "K7s"_ast_re, "K6s"_ast_re, "K5s"_ast_re, "K4s"_ast_re, "K3s"_ast_re, "K2s"_ast_re,
    "AQo"_ast_re, "KQo"_ast_re, "QQ"_ast_re, "QJs"_ast_re, "QTs"_ast_re, "Q9s"_ast_re, "Q8s"_ast_re, "Q7s"_ast_re, "Q6s"_ast_re, "Q5s"_ast_re, "Q4s"_ast_re, "Q3s"_ast_re, "Q2s"_ast_re,
    "AJo"_ast_re, "KJo"_ast_re, "QJo"_ast_re, "JJ"_ast_re, "JTs"_ast_re, "J9s"_ast_re, "J8s"_ast_re, "J7s"_ast_re, "J6s"_ast_re, "J5s"_ast_re, "J4s"_ast_re, "J3s"_ast_re, "J2s"_ast_re,
    "ATo"_ast_re, "KTo"_ast_re, "QTo"_ast_re, "JTo"_ast_re, "TT"_ast_re, "T9s"_ast_re, "T8s"_ast_re, "T7s"_ast_re, "T6s"_ast_re, "T5s"_ast_re, "T4s"_ast_re, "T3s"_ast_re, "T2s"_ast_re,
    "A9o"_ast_re, "K9o"_ast_re, "Q9o"_ast_re, "J9o"_ast_re, "T9o"_ast_re, "99"_ast_re, "98s"_ast_re, "97s"_ast_re, "96s"_ast_re, "95s"_ast_re, "94s"_ast_re, "93s"_ast_re, "92s"_ast_re,
    "A8o"_ast_re, "K8o"_ast_re, "Q8o"_ast_re, "J8o"_ast_re, "T8o"_ast_re, "98o"_ast_re, "88"_ast_re, "87s"_ast_re, "86s"_ast_re, "85s"_ast_re, "84s"_ast_re, "83s"_ast_re, "82s"_ast_re,
    "A7o"_ast_re, "K7o"_ast_re, "Q7o"_ast_re, "J7o"_ast_re, "T7o"_ast_re, "97o"_ast_re, "87o"_ast_re, "77"_ast_re, "76s"_ast_re, "75s"_ast_re, "74s"_ast_re, "73s"_ast_re, "72s"_ast_re,
    "A6o"_ast_re, "K6o"_ast_re, "Q6o"_ast_re, "J6o"_ast_re, "T6o"_ast_re, "96o"_ast_re, "86o"_ast_re, "76o"_ast_re, "66"_ast_re, "65s"_ast_re, "64s"_ast_re, "63s"_ast_re, "62s"_ast_re,
    "A5o"_ast_re, "K5o"_ast_re, "Q5o"_ast_re, "J5o"_ast_re, "T5o"_ast_re, "95o"_ast_re, "85o"_ast_re, "75o"_ast_re, "65o"_ast_re, "55"_ast_re, "54s"_ast_re, "53s"_ast_re, "52s"_ast_re,
    "A4o"_ast_re, "K4o"_ast_re, "Q4o"_ast_re, "J4o"_ast_re, "T4o"_ast_re, "94o"_ast_re, "84o"_ast_re, "74o"_ast_re, "64o"_ast_re, "54o"_ast_re, "44"_ast_re, "43s"_ast_re, "42s"_ast_re,
    "A3o"_ast_re, "K3o"_ast_re, "Q3o"_ast_re, "J3o"_ast_re, "T3o"_ast_re, "93o"_ast_re, "83o"_ast_re, "73o"_ast_re, "63o"_ast_re, "53o"_ast_re, "43o"_ast_re, "33"_ast_re, "32s"_ast_re,
    "A2o"_ast_re, "K2o"_ast_re, "Q2o"_ast_re, "J2o"_ast_re, "T2o"_ast_re, "92o"_ast_re, "82o"_ast_re, "72o"_ast_re, "62o"_ast_re, "52o"_ast_re, "42o"_ast_re, "32o"_ast_re, "22"_ast_re,
  };
    std::vector<prc::parser::ast::hand> ret;
    for (auto const& elem : elems)
    {
      ret.emplace_back(boost::get<prc::parser::ast::hand>(elem));
    }
    return ret;
  }();
  // clang-format on
  return hands;
}

// it's written in this order in the file, not a big-endian number but using
// big_word allows to write bytes in the same order
auto const separator = x3::omit[x3::big_dword(0x39300000)];

struct utf16_to_utf8_string_parser : x3::parser<utf16_to_utf8_string_parser>
{
  using attribute_type = std::string;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    if (!x3::parse(it, last, x3::big_dword(0x65000000) >> x3::big_word(0xfffe)))
    {
      return false;
    }
    std::vector<char> buf;
    if (!x3::parse(it,
                   last,
                   // max gto+ size is 30 000, so max 2 bytes
                   x3::repeat(1, 2)[x3::char_(0xff)],
                   buf))
    {
      return false;
    }
    auto const encoded_size_bytes = buf.size();
    buf.clear();
    if (!x3::parse(it, last, x3::repeat(encoded_size_bytes)[x3::char_], buf))
      return false;
    auto nb_utf16_code_units = 0u;
    std::memcpy(&nb_utf16_code_units, buf.data(), buf.size());
    std::vector<char16_t> utf16_buf;
    if (!x3::parse(it,
                   last,
                   x3::repeat(nb_utf16_code_units)[x3::little_word],
                   utf16_buf))
      return false;
    attr = detail::utf16le_to_utf8(
        std::u16string_view(utf16_buf.data(), utf16_buf.size()));
    first = it;
    return true;
  }
};

inline constexpr utf16_to_utf8_string_parser utf16_to_utf8_string;

struct info_parser : x3::parser<info_parser>
{
  using attribute_type = ast::info;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    std::string name;
    if (!x3::parse(it, last, separator >> utf16_to_utf8_string, name))
      return false;
    if (!x3::parse(it, last, x3::little_dword(0x00)))
      return false;
    std::pair<unsigned char, unsigned int> p;
    if (!x3::parse(it, last, x3::byte_ >> x3::little_dword, p))
      return false;
    ast::entry_type e;
    auto [first_nb, second_nb] = p;
    if (first_nb > 1 || second_nb > 2)
      return false;
    else if (first_nb == 0)
    {
      if (second_nb == 1)
        e = ast::entry_type::range;
      else
        return false;
    }
    else
    {
      if (second_nb == 1)
        e = ast::entry_type::range;
      else if (second_nb == 0)
        e = ast::entry_type::category;
      else
        e = ast::entry_type::group;
    }
    std::pair<int, int> p2;
    if (!x3::parse(it, last, x3::little_dword >> x3::little_dword, p2))
    {
      return false;
    }
    first = it;
    attr = ast::info{std::move(name), e, p2.first, p2.second};
    return true;
  }
};

inline constexpr info_parser _info;

struct category_parser : x3::parser<category_parser>
{
  using attribute_type = ast::category;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    ast::info info;
    if (!x3::parse(it, last, _info, info))
      return false;
    if (info.type != ast::entry_type::category)
      return false;
    attr = ast::category{std::move(info)};
    first = it;
    return true;
  }
};

inline constexpr category_parser _category;

auto const default_weight_cb = [](auto& ctx) {
  _val(ctx).weight = 100.0;
  _val(ctx).elems = std::move(_attr(ctx));
};

auto const check_weights_cb = [](auto& ctx) {
  double w1, w2;
  std::vector<prc::parser::ast::range_elem> elems;
  boost::fusion::tie(w1, elems, w2) = _attr(ctx);
  if (w1 != w2)
    _pass(ctx) = false;
  else
  {
    _val(ctx).elems = std::move(elems);
    _val(ctx).weight = w1;
  }
};

auto const _weighted_elems = prc::parser::as<ast::weighted_elems>[(
    x3::lit('[') > x3::double_ > ']' > (prc::parser::range_elem() % ',') >
    "[/" > x3::double_ > ']')[check_weights_cb]];

auto const _no_weight_elems = prc::parser::as<ast::weighted_elems>[(
    prc::parser::range_elem() % ',')[default_weight_cb]];

struct group_ratios_parser : x3::parser<group_ratios_parser>
{
  using attribute_type = std::vector<ast::group_ratio>;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    auto nb_groups = 0;
    // first little_dword can be 0x84 or 0x80, I don't know what those mean
    // (most likely bitfield), so just skip
    if (!x3::parse(it,
                   last,
                   x3::omit[x3::little_dword] >> x3::little_dword,
                   nb_groups))
    {
      return false;
    }
    if (nb_groups == 0)
      return false;
    std::vector<int> group_indexes;
    if (!x3::parse(
            it, last, x3::repeat(nb_groups)[x3::little_dword], group_indexes))
    {
      return false;
    }
    std::vector<std::vector<unsigned char>> radix_floats;
    if (!x3::parse(it,
                   last,
                   x3::repeat(nb_groups)[x3::repeat(8)[x3::byte_]],
                   radix_floats))
    {
      return false;
    }
    std::vector<double> ratios(radix_floats.size());
    for (auto i = 0; i < nb_groups; ++i)
      std::memcpy(&ratios[i], radix_floats[i].data(), 8);
    attr.resize(nb_groups);
    for (auto i = 0; i < nb_groups; ++i)
      attr[i] = {group_indexes[i], ratios[i]};
    first = it;
    return true;
  }
};

inline constexpr group_ratios_parser group_ratios;

struct all_hand_info_parser : x3::parser<all_hand_info_parser>
{
  using attribute_type = std::vector<ast::hand_info>;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    std::vector<std::vector<ast::group_ratio>> ratios;
    auto const& hands = sorted_hands();
    assert(hands.size() == 169);
    if (!x3::parse(it, last, x3::repeat(hands.size())[group_ratios], ratios))
      return false;
    attr.resize(hands.size());
    for (auto i = 0; i < hands.size(); ++i)
      attr[i] = ast::hand_info{hands[i], ratios[i]};
    first = it;
    return true;
  }
};

inline constexpr all_hand_info_parser all_hand_info;

struct range_parser : x3::parser<range_parser>
{
  using attribute_type = ast::range;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    ast::info info;
    if (!x3::parse(it, last, _info, info))
      return false;
    if (info.type != ast::entry_type::range)
      return false;
    std::string range_content;
    std::vector<ast::info> group_info;
    if (!x3::parse(it, last, x3::repeat(info.nb_subentries)[_info], group_info))
      return false;
    if (!x3::parse(it, last, utf16_to_utf8_string, range_content))
      return false;
    // this horror seems needed, haven't found a better way of reusing the
    // context
    std::vector<ast::weighted_elems> elems;
    if (!x3::phrase_parse(range_content.cbegin(),
                          range_content.cend(),
                          x3::with<x3::error_handler_tag>(context.get(
                              boost::mpl::identity<x3::error_handler_tag>{}))
                              [(_weighted_elems | _no_weight_elems) % ','],
                          x3::space,
                          elems))
    {
      return false;
    }
    std::vector<ast::hand_info> hand_info;
    if (!x3::parse(it, last, all_hand_info, hand_info))
      return false;
    // group indexes are repeated at the end, TODO check invalid
    int nb_groups = 0;
    if (!x3::parse(it, last, x3::little_dword, nb_groups))
      return false;
    if (nb_groups != group_info.size())
      return false;
    if (!x3::parse(it, last, x3::omit[x3::repeat(nb_groups)[x3::little_dword]]))
      return false;
    first = it;
    attr = ast::range{std::move(info),
                      std::move(group_info),
                      std::move(elems),
                      std::move(hand_info)};
    return true;
  }
};

inline constexpr range_parser _range;

file_type const _file_type = "file";
auto const _file_type_def = x3::no_skip[+(_category | _range) > x3::eoi];

BOOST_SPIRIT_DEFINE(_file_type);

struct file_class : error_handler
{
};
}
