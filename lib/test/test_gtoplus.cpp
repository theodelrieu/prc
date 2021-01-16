#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/locale.hpp>
#include <catch2/catch.hpp>

#include <prc/gtoplus/parser/api.hpp>

extern std::string testDataPath;

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;
using namespace prc::parser::ast::literals;
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
    auto ctx = init_context(content, gtoplus::parser::file());
    auto begin = content.begin();
    auto const end = content.end();

    std::vector<gtoplus::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(begin, end, ctx, x3::space, entries);
    REQUIRE(r);
    REQUIRE(begin == end);

    REQUIRE(entries.size() == 1);
    auto& entry = entries.front();
    REQUIRE(entry.which() == 0);
    auto& category = boost::get<gtoplus::parser::ast::category>(entry);

    CHECK(category.info.name == "default_name");
    CHECK(category.info.type == gtoplus::parser::ast::entry_type::category);
    CHECK(category.info.nb_subentries == 0);
  }

  SECTION("Aces")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "gtoplus" / "aces.txt");
    auto ctx = init_context(content, gtoplus::parser::file());
    auto begin = content.begin();
    auto const end = content.end();

    std::vector<gtoplus::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(begin, end, ctx, x3::space, entries);
    REQUIRE(r);
    REQUIRE(begin == end);

    REQUIRE(entries.size() == 2);
    auto& category =
        boost::get<gtoplus::parser::ast::category>(entries.front());
    auto& range = boost::get<gtoplus::parser::ast::range>(entries.back());

    CHECK(range.info.name == "Aces");
    CHECK(range.info.type == gtoplus::parser::ast::entry_type::range);
    CHECK(range.info.nb_subentries == 0);
    REQUIRE(range.weighted_elems.size() == 1);
    CHECK(range.weighted_elems.front().elems == std::vector{"AA"_ast_re});
    CHECK(range.weighted_elems.front().weight == 100.0);
  }

  // SECTION("Weights")
  // {
  //   auto const content =
  //       read_all(fs::path{testDataPath} / "gtoplus" / "weights.txt");
  //   auto ctx = init_context(content, gtoplus::parser::file());
  //   auto begin = content.begin();
  //   auto const end = content.end();
  //
  //   std::vector<gtoplus::parser::ast::entry> entries;
  //   auto const r = x3::phrase_parse(begin, end, ctx, x3::space, entries);
  //   REQUIRE(r);
  //   REQUIRE(begin == end);
  //
  //   REQUIRE(entries.size() == 2);
  //   auto& category =
  //       boost::get<gtoplus::parser::ast::category>(entries.front());
  //   auto& range = boost::get<gtoplus::parser::ast::range>(entries.back());
  //
  //   CHECK(range.info.name == "weights");
  //   CHECK(range.info.type == gtoplus::parser::ast::entry_type::range);
  //   CHECK(range.info.nb_subentries == 0);
  //   REQUIRE(range.elems.size() == 1);
  //   CHECK(range.elems.front() == "AA"_ast_re);
  // }
}
