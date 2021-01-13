#pragma once

#define BOOST_SPIRIT_X3_DEBUG
#include <boost/locale.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include <boost/variant.hpp>

#include <prc/gtoplus/parser/api.hpp>
#include <prc/gtoplus/parser/ast.hpp>
#include <prc/gtoplus/parser/ast_adapted.hpp>
#include <prc/gtoplus/parser/config.hpp>
#include <prc/parser/as_type.hpp>

#include <cstring>

namespace prc::gtoplus::parser
{
struct error_handler
{
  template <typename Iterator, typename Exception, typename Context>
  x3::error_handler_result on_error(Iterator& first,
                                    Iterator const& last,
                                    Exception const& x,
                                    Context const& context)
  {
    using namespace std::string_literals;

    auto& error_handler = x3::get<x3::error_handler_tag>(context);
    auto msg = "Expecting "s + x.which() + " here:";
    error_handler(x.where(), msg);
    return x3::error_handler_result::fail;
  }
};

unsigned encoded_size_bytes;
unsigned encoded_size;
struct _encoded_size_bytes;
struct _encoded_size;

auto const separator =
    x3::lexeme[x3::lit('\x39') >> '\x30' >> '\x00' >> '\x00'];
auto const utf16_string_header =
    x3::lexeme[x3::lit('\x65') >> '\x00' >> '\x00' >> '\x00' >> '\xff' >>
               '\xfe'];

auto const encoded_size_bytes_cb = [](auto& ctx) {
  std::cout << "encoded_size_bytes = " << _attr(ctx).size() << std::endl;
  x3::get<_encoded_size_bytes>(ctx) = _attr(ctx).size();
};
auto const encoded_size_cb = [](auto& ctx) {
  auto& vec = _attr(ctx);
  if (vec.empty() || vec.size() > 2)
    _pass(ctx) = false;
  else
  {
    std::uint16_t size = 0;
    std::memcpy(&size, vec.data(), vec.size());
    x3::get<_encoded_size>(ctx) = size;
  }
};
auto const utf16_to_utf8_cb = [](auto& ctx) {
  auto& vec = _attr(ctx);
  _val(ctx) = boost::locale::conv::utf_to_utf<char>(vec.data(),
                                                    vec.data() + vec.size());
};

auto const bytes_to_entry_type = [](auto& ctx) {
  auto& vec = _attr(ctx);
  if (vec[0] > 1 || vec[1] > 2)
    _pass(ctx) = false;
  else if (vec[0] == 0)
  {
    if (vec[1] == 1)
      _val(ctx) = ast::entry_type::grouped_range;
    else
      _pass(ctx) = false;
  }
  else if (vec[1] == 1)
    _val(ctx) = ast::entry_type::range;
  else
    _val(ctx) = ast::entry_type::category;
};

// might need std::ref
auto const utf8_string =
    prc::parser::as<std::string>[x3::with<_encoded_size_bytes>(
        encoded_size_bytes)[x3::with<_encoded_size>(
        encoded_size)[utf16_string_header > *x3::char_]]];
// x3::repeat(1, 2)[x3::char_("\xff")][encoded_size_bytes_cb] >
// x3::repeat(encoded_size_bytes)[x3::char_][encoded_size_cb] >
// x3::repeat(encoded_size * 2)[x3::char_][utf16_to_utf8_cb]]]];

auto const entry_type = prc::parser::as<ast::entry_type>[x3::repeat(
    2)[x3::char_][bytes_to_entry_type]];

info_type const _info = "info";
auto const _info_def = separator >> utf8_string >>
                       x3::repeat(4)[x3::lit("\x00")] >> entry_type
                       >> x3::repeat(3)[x3::lit("\x00")] >> x3::little_dword;

BOOST_SPIRIT_DEFINE(_info);

struct info_class : error_handler
{
};
}
