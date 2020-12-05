#pragma once

// #define BOOST_SPIRIT_X3_DEBUG
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include <boost/variant.hpp>

#include <prc/equilab/parser/api.hpp>
#include <prc/equilab/parser/ast.hpp>
#include <prc/equilab/parser/ast_adapted.hpp>
#include <prc/equilab/parser/config.hpp>
#include <prc/parser/api.hpp>
#include <prc/parser/as_type.hpp>

namespace prc::equilab::parser
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

group_info_type const _group_info = "group info";
weighted_hands_type const _weighted_hands = "weighted hands";
group_type const _group = "group";
range_type const _range = "range";
folder_type const _folder = "folder";
file_type const _file = "file";

auto const fill_any_two = [](auto& ctx) {
  using namespace prc::parser::ast::literals;
  _val(ctx) = std::vector{
      "22+"_ast_re,  "A2o+"_ast_re, "K2o+"_ast_re, "Q2o+"_ast_re, "J2o+"_ast_re,
      "T2o+"_ast_re, "92o+"_ast_re, "82o+"_ast_re, "72o+"_ast_re, "62o+"_ast_re,
      "52o+"_ast_re, "42o+"_ast_re, "32o"_ast_re,  "A2s+"_ast_re, "K2s+"_ast_re,
      "Q2s+"_ast_re, "J2s+"_ast_re, "T2s+"_ast_re, "92s+"_ast_re, "82s+"_ast_re,
      "72s+"_ast_re, "62s+"_ast_re, "52s+"_ast_re, "42s+"_ast_re, "32s"_ast_re};
};

auto const depth_str_to_int = [](auto& ctx) { _val(ctx) = _attr(ctx).size(); };
auto const default_weight = [](auto& ctx) {
  _val(ctx) = _attr(ctx).value_or(100.0);
};
auto const trim = [](auto& ctx) { boost::trim(_attr(ctx)); };

auto const rgb_string_to_int = [](auto& ctx) {
  // no std::stoi(begin, end) :(
  auto& str = _attr(ctx);
  auto r = 100 * (str[0] - '0') + 10 * (str[1] - '0') + (str[2] - '0');
  auto g = 100 * (str[3] - '0') + 10 * (str[4] - '0') + (str[5] - '0');
  auto b = 100 * (str[6] - '0') + 10 * (str[7] - '0') + (str[8] - '0');
  _val(ctx) = 0xff000000 | (r << 16) | (g << 8) | b;
};
auto const utf32_to_utf8 = [](auto& ctx) {
  auto& vec = _attr(ctx);
  _val(ctx) = boost::locale::conv::utf_to_utf<char>(vec.data(),
                                                    vec.data() + vec.size());
  boost::trim(_val(ctx));
};

auto const _delim = x3::unicode::lit(U"\U000000B0");

auto const _depth = prc::parser::as<
    int>[x3::lexeme[+x3::unicode::char_(U".")][depth_str_to_int]];

auto const _weight = prc::parser::as<double>[(
    -(x3::double_ >> x3::unicode::lit(U":")))[default_weight]];
auto const _any_two =
    prc::parser::as<std::vector<prc::parser::ast::range_elem>>[x3::unicode::lit(
        U"random")[fill_any_two]];

auto const _rgb = prc::parser::as<int>[(
    x3::repeat(9)[x3::unicode::digit])[rgb_string_to_int]];

auto const _range_name = prc::parser::as<std::string>[x3::lexeme[+(
    x3::unicode::print - x3::unicode::lit(U"{"))][utf32_to_utf8]];
auto const _group_name = prc::parser::as<
    std::string>[x3::lexeme[*(x3::unicode::print - _delim)][utf32_to_utf8]];

auto const _folder_name = prc::parser::as<
    std::string>[x3::lexeme[+(x3::unicode::print - x3::unicode::lit(U"{")) >>
                            x3::eol][utf32_to_utf8]];

// both hands and weights have the same ',' separator, which makes the grammar
// definition a bit more complex than (range_elem() % ',')
// the problem arises when there's something like:
// 100: AKs, 99.99: JJ+
// with the native approach, 99.99 will be parsed as a hand
auto const _workaround = prc::parser::range_elem() >>
                         *(x3::unicode::lit(U",") >>
                           prc::parser::range_elem() >>
                           !(x3::unicode::lit(U":") |
                             x3::unicode::lit(U"."))) >>
                         -x3::unicode::lit(U",");

auto const _note =
    x3::unicode::lit(U"{") >>
    prc::parser::as<std::string>[x3::lexeme[+(
        x3::unicode::print - x3::unicode::lit(U"}"))][utf32_to_utf8]] >
    x3::unicode::lit(U"}");

auto const _weighted_hands_def = -_weight >> (_any_two | _workaround);
auto const _group_info_def = _delim >> x3::int_ >> _delim >> x3::int_ >>
                             _delim >> _rgb >> _delim >> _group_name >>
                             _delim >> x3::int_ >> _delim;

auto const _group_def = +_weighted_hands >> -_group_info;

auto const _range_def = _depth >> _range_name >> x3::unicode::lit(U"{") >>
                        *_group >> x3::unicode::lit(U"}") >> -_note;

auto const _folder_def = _depth >> _folder_name;

auto const _file_def = x3::unicode::lit(U"[Userdefined]") >>
                       *(_range | _folder);

BOOST_SPIRIT_DEFINE(
    _weighted_hands, _group_info, _group, _range, _folder, _file);

struct group_info_class : error_handler
{
};

struct group_class : error_handler
{
};

struct range_class : error_handler
{
};

struct folder_class : error_handler
{
};

struct file_class : error_handler
{
};
}
