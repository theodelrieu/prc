#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/locale.hpp>
#include <catch2/catch.hpp>

#include <prc/gtoplus/parser/api.hpp>

extern std::string testDataPath;

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;
using namespace prc;

namespace
{
template <typename Parser>
auto init_context(std::string const& input,
                  Parser p,
                  std::ostream& error_stream = std::cerr)
{
  x3::error_handler<std::string::const_iterator> error_handler(
      input.begin(), input.end(), error_stream);

  return x3::with<x3::error_handler_tag>(std::move(error_handler))[p];
}

std::string read_all(fs::path const& p)
{
  if (!fs::exists(p))
    throw std::runtime_error("No such path: " + p.string());
  std::ifstream ifs{p, std::ios::binary};
  return std::string{std::istreambuf_iterator<char>(ifs), {}};
}
}

TEST_CASE("gtoplus format tests", "[gtoplus]")
{
  SECTION("Empty file")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "gtoplus" / "empty.txt");
    auto ctx = init_context(content, gtoplus::parser::info());

    gtoplus::parser::ast::info info;
    auto const r =
        x3::phrase_parse(content.begin(), content.end(), ctx, x3::space, info);
    REQUIRE(r);

    CHECK(info.name == "default_name");
    CHECK(info.type == gtoplus::parser::ast::entry_type::category);
    CHECK(info.nb_subentries == 0);
  }
}
