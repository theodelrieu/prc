#pragma once

#include <boost/fusion/include/adapt_struct.hpp>

#include <prc/equilab/parser/ast.hpp>

BOOST_FUSION_ADAPT_STRUCT(prc::equilab::parser::ast::group_info,
                          index,
                          parent_index,
                          rgb,
                          name,
                          nesting_index)
BOOST_FUSION_ADAPT_STRUCT(prc::equilab::parser::ast::weighted_hands,
                          weight,
                          hands)
BOOST_FUSION_ADAPT_STRUCT(prc::equilab::parser::ast::group,
                          weighted_hands,
                          info)
BOOST_FUSION_ADAPT_STRUCT(
    prc::equilab::parser::ast::range, depth, name, groups, note)
BOOST_FUSION_ADAPT_STRUCT(prc::equilab::parser::ast::folder, depth, name)
