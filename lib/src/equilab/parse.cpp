#include <prc/equilab/parse.hpp>

#include <prc/detail/unicode.hpp>
#include <prc/equilab/parser/api.hpp>

#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace prc::equilab
{
folder parse(fs::path const& src_file)
{
  namespace x3 = boost::spirit::x3;

  std::ifstream ifs{src_file.string(), std::ios::binary};
  std::string content(std::istreambuf_iterator<char>(ifs), {});
  auto const utf32 = detail::utf16le_to_utf32(std::u16string_view{
      reinterpret_cast<char16_t const*>(content.data()), content.size() / 2});

  x3::error_handler<std::u32string::const_iterator> error_handler(
      utf32.begin(), utf32.end(), std::cerr);

  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[equilab::parser::file()];

  std::vector<equilab::parser::ast::entry> entries;
  auto b = utf32.begin();
  auto e = utf32.end();
  auto r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
  if (!r || b != e)
    throw std::runtime_error("failed to parse");
  return prc::folder{"/", entries};
}
}
