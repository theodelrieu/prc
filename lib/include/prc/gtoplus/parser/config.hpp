#pragma once

#include <prc/parser/config.hpp>

#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace prc::gtoplus::parser
{
namespace x3 = boost::spirit::x3;

template <typename Iterator>
using context_type =
    x3::context<x3::error_handler_tag,
                x3::error_handler<Iterator>,
                typename x3::phrase_parse_context<x3::space_type>::type>;
}
