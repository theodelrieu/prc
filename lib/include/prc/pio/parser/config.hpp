#pragma once

#include <prc/parser/config.hpp>

namespace prc::pio::parser
{
namespace x3 = boost::spirit::x3;
template <typename Iterator>
using context_type = prc::parser::context_type<Iterator>;
}