#pragma once

#include <optional>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace prc::parser
{
namespace x3 = boost::spirit::x3;

template <typename Iterator, typename Skipper>
using context_type =
    x3::context<x3::error_handler_tag,
                x3::error_handler<Iterator>,
                typename x3::phrase_parse_context<Skipper>::type>;
}

namespace boost::spirit::x3::traits
{
template <typename T>
struct is_optional<std::optional<T>> : mpl::true_
{
};
}
