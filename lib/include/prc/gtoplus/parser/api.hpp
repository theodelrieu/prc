#pragma once

#include <prc/gtoplus/parser/ast.hpp>
#include <prc/gtoplus/parser/config.hpp>

#include <boost/spirit/home/x3.hpp>

namespace prc::gtoplus::parser
{
using info_type = x3::rule<struct info_class, ast::info>;

BOOST_SPIRIT_DECLARE(info_type);

info_type info();
}
