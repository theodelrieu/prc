#pragma once

#include <prc/gtoplus/parser/ast.hpp>
#include <prc/gtoplus/parser/config.hpp>

#include <boost/spirit/home/x3.hpp>

namespace prc::gtoplus::parser
{
using file_type = x3::rule<struct file_class, std::vector<ast::entry>>;

BOOST_SPIRIT_DECLARE(file_type);

file_type file();
}
