#pragma once

//#define BOOST_SPIRIT_X3_DEBUG
#include <boost/spirit/home/x3.hpp>
#include <boost/variant.hpp>

#include <prc/pio/parser/api.hpp>
#include <prc/pio/parser/ast.hpp>
#include <prc/pio/parser/ast_adapted.hpp>
#include <prc/pio/parser/config.hpp>

namespace prc::pio::parser
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

x3::symbols_parser<boost::spirit::char_encoding::unicode, bool> const included(
    {{U"True", true}, {U"False", false}}, "included");
base_range_type const _base_range = "base range";
subrange_type const _subrange = "subrange";
range_type const _range = "range";

struct subrange_name_class;
x3::rule<subrange_name_class, std::string> const _subrange_name =
    "subrange_name";

auto const base_range_name =
    x3::lexeme[+(x3::unicode::char_ - x3::eol) > x3::eol];
auto const _subrange_name_def =
    x3::lexeme[+(x3::unicode::char_ - x3::unicode::char_(U"\t"))];

auto const _base_range_def = base_range_name > x3::repeat(1326)[x3::double_];
auto const _subrange_def = included > x3::int_ > _subrange_name >
                           x3::repeat(1326)[x3::double_];

auto const _range_def = (x3::expect[base_range()] >> *subrange()) > x3::eoi;

BOOST_SPIRIT_DEFINE(_base_range, _subrange, _subrange_name, _range);

struct base_range_class : error_handler
{
};

struct subrange_class : error_handler
{
};

struct range_class : error_handler
{
};

struct subrange_name_class : error_handler
{
};
}
