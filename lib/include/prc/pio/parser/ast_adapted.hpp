#pragma once

#include <boost/fusion/include/adapt_struct.hpp>

#include <prc/pio/parser/ast.hpp>

BOOST_FUSION_ADAPT_STRUCT(prc::pio::parser::ast::base_range, name, weights)
BOOST_FUSION_ADAPT_STRUCT(
    prc::pio::parser::ast::subrange, included, rgb, name, weights)
BOOST_FUSION_ADAPT_STRUCT(prc::pio::parser::ast::range, base_range, subranges)
