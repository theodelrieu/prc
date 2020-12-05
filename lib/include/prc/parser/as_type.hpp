#pragma once

#include <boost/spirit/home/x3.hpp>

namespace prc::parser
{
template <typename T>
struct as_type
{
  template <typename E>
  constexpr auto operator[](E e) const
  {
    return boost::spirit::x3::rule<struct _, T>{} = e;
  }
};

template <typename T>
static inline constexpr as_type<T> as;
}
