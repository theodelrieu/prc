#pragma once

#include <prc/parser/ast.hpp>
#include <prc/parser/config.hpp>

#include <boost/spirit/home/support/char_encoding/unicode.hpp>
#include <boost/spirit/home/x3.hpp>

namespace prc::parser
{
namespace x3 = boost::spirit::x3;

struct card_class;
struct combo_class;
struct unpaired_hand_class;
struct hand_class;
struct hand_range_class;
struct range_elem_class;

using card_type = x3::rule<card_class, ast::card>;
using combo_type = x3::rule<combo_class, ast::combo>;
using unpaired_hand_type = x3::rule<unpaired_hand_class, ast::unpaired_hand>;
using hand_type = x3::rule<hand_class, ast::hand>;
using hand_range_type = x3::rule<hand_range_class, ast::hand_range>;
using range_elem_type = x3::rule<range_elem_class, ast::range_elem>;

using suit_type =
    x3::symbols_parser<boost::spirit::char_encoding::unicode, prc::suit>;
using rank_type =
    x3::symbols_parser<boost::spirit::char_encoding::unicode, prc::rank>;
using pair_type =
    x3::symbols_parser<boost::spirit::char_encoding::unicode, ast::paired_hand>;

card_type card();
combo_type combo();
unpaired_hand_type unpaired_hand();
hand_type hand();
hand_range_type hand_range();
range_elem_type range_elem();

suit_type const& suit();
rank_type const& rank();
pair_type const& paired_hand();
}

#include <prc/parser/api_def.hpp>
