#include <prc/equilab/parse.hpp>

#include <prc/equilab/parser/api.hpp>

#include <boost/locale.hpp>

#include <fstream>

namespace fs = std::filesystem;

namespace prc::equilab
{
folder parse(fs::path const& src_file)
{
  namespace x3 = boost::spirit::x3;

  std::ifstream ifs(src_file.string(), std::ios::binary);
  std::string const utf16(std::istreambuf_iterator<char>(ifs), {});

  auto const utf8 = boost::locale::conv::between(utf16, "UTF8", "UTF16-LE");
  auto const utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8);
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
