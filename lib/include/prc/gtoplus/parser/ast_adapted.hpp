#pragma once

#include <boost/fusion/include/adapt_struct.hpp>

#include <prc/gtoplus/parser/ast.hpp>

BOOST_FUSION_ADAPT_STRUCT(prc::gtoplus::parser::ast::info,
                          name,
                          type,
                          nb_subentries);
