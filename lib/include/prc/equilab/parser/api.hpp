#pragma once

#include <prc/equilab/parser/ast.hpp>
#include <prc/equilab/parser/config.hpp>

#include <boost/spirit/home/x3.hpp>

namespace prc::equilab::parser
{
struct group_info_class;
struct weighted_hands_class;
struct group_class;
struct range_class;
struct folder_class;
struct file_class;

using group_info_type = x3::rule<group_info_class, ast::group_info>;
using weighted_hands_type = x3::rule<weighted_hands_class, ast::weighted_hands>;
using group_type = x3::rule<group_class, ast::group>;
using range_type = x3::rule<range_class, ast::range>;
using folder_type = x3::rule<folder_class, ast::folder>;
using file_type = x3::rule<file_class, std::vector<ast::entry>>;

BOOST_SPIRIT_DECLARE(group_info_type,
                     weighted_hands_type,
                     group_type,
                     range_type,
                     folder_type,
                     file_type);

file_type file();
range_type range();
}
