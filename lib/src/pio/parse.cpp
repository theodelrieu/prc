#include <prc/pio/parse.hpp>

#include <prc/detail/unicode.hpp>
#include <prc/pio/parser/api.hpp>

#include <boost/spirit/home/x3.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;

namespace prc::pio
{
namespace
{
std::u32string read_all(fs::path const& p)
{
  if (!fs::exists(p))
    throw std::runtime_error("No such path: " + p.string());
  if (p.extension() != ".txt")
    throw std::runtime_error("invalid pio range path: " + p.string());
  std::ifstream ifs{p};
  std::string content(std::istreambuf_iterator<char>(ifs), {});
  return detail::utf8_to_utf32(content);
}
}

prc::range parse(fs::path const& pio_range_path)
{
  auto const content = read_all(pio_range_path);

  x3::error_handler<std::u32string::const_iterator> error_handler(
      content.begin(), content.end(), std::cerr);

  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[x3::expect[parser::range()] > x3::eoi];
  parser::ast::range r;
  x3::phrase_parse(content.begin(), content.end(), ctx, x3::unicode::space, r);
  return prc::range{r};
}
}
