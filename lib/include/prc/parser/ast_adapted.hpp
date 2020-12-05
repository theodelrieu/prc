#pragma once

#include <boost/fusion/include/adapt_struct.hpp>

#include <prc/parser/ast.hpp>

BOOST_FUSION_ADAPT_STRUCT(prc::parser::ast::card, rank, suit)
BOOST_FUSION_ADAPT_STRUCT(prc::parser::ast::combo, first_card, second_card)
BOOST_FUSION_ADAPT_STRUCT(prc::parser::ast::unpaired_hand,
                          first_rank,
                          second_rank,
                          suitedness)
BOOST_FUSION_ADAPT_STRUCT(prc::parser::ast::paired_hand, rank)
BOOST_FUSION_ADAPT_STRUCT(prc::parser::ast::hand_range, from, to)
