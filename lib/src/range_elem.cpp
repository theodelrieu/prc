#include <prc/range_elem.hpp>

#include <prc/detail/unicode.hpp>

#include <iostream>

namespace prc
{
range_elem::range_elem(hand const& h) : _v(h)
{
}

range_elem::range_elem(combo const& c) : _v(c)
{
}

range_elem::range_elem(hand_range const& hr) : _v(hr)
{
}

range_elem::range_elem(parser::ast::range_elem const& re)
{
  if (auto p = boost::get<parser::ast::hand>(&re))
    _v = hand{*p};
  else if (auto p = boost::get<parser::ast::hand_range>(&re))
    _v = hand_range{*p};
  else
    _v = combo{boost::get<parser::ast::combo>(re)};
}

std::string range_elem::string() const
{
  return boost::variant2::visit([](auto& e) { return e.string(); }, _v);
}

int range_elem::index() const
{
  return _v.index();
}

bool operator==(range_elem const& lhs, range_elem const& rhs)
{
  return lhs._v == rhs._v;
}

bool operator!=(range_elem const& lhs, range_elem const& rhs)
{
  return !(lhs == rhs);
}

bool operator<(range_elem const& lhs, range_elem const& rhs)
{
  return lhs._v < rhs._v;
}

std::ostream& operator<<(std::ostream& os, range_elem const& h)
{
  return os << h.string();
}

inline namespace literals
{
range_elem operator"" _re(char const* str, std::size_t n)
{
  namespace x3 = boost::spirit::x3;

  auto const s = detail::utf8_to_utf32(std::string_view{str, n});

  x3::error_handler<std::u32string::const_iterator> error_handler(
      s.begin(), s.end(), std::cerr);
  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[x3::expect[parser::range_elem()] > x3::eoi];
  parser::ast::range_elem c;
  x3::phrase_parse(s.begin(), s.end(), ctx, x3::unicode::space, c);
  return range_elem{c};
}
}
}
