#pragma once

#include <prc/pio/parser/ast.hpp>
#include <prc/pio/parser/config.hpp>

#include <boost/spirit/home/x3.hpp>

namespace prc::pio::parser
{
struct base_range_class;
struct subrange_class;
struct range_class;

using base_range_type = x3::rule<base_range_class, ast::base_range>;
using subrange_type = x3::rule<subrange_class, ast::subrange>;
using range_type = x3::rule<range_class, ast::range>;

BOOST_SPIRIT_DECLARE(base_range_type, subrange_type, range_type);

base_range_type base_range();
subrange_type subrange();
range_type range();
}
