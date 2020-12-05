#pragma once

#include <optional>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace prc::parser
{
namespace x3 = boost::spirit::x3;

template <typename Iterator>
using context_type =
    x3::context<x3::error_handler_tag,
                x3::error_handler<Iterator>,
                x3::phrase_parse_context<x3::unicode::space_type>::type>;
}

namespace boost::spirit::x3::traits
{
template <typename T>
struct is_optional<std::optional<T>> : mpl::true_
{
};
}
