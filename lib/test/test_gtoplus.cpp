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
    CHECK(range.groups.empty());
    CHECK(range.info.type == gtoplus::parser::ast::entry_type::range);
    CHECK(range.info.nb_subentries == 0);
    REQUIRE(range.weighted_elems.size() == 1);
    CHECK(range.weighted_elems.front().elems == std::vector{"AA"_ast_re});
    CHECK(range.weighted_elems.front().weight == 100.0);
  }

  SECTION("Weights")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "gtoplus" / "weights.txt");
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

    CHECK(range.info.name == "weights");
    CHECK(range.info.type == gtoplus::parser::ast::entry_type::range);
    CHECK(range.info.nb_subentries == 0);
    CHECK(range.groups.empty());
    REQUIRE(range.weighted_elems.size() == 2);
    CHECK(range.weighted_elems[0].elems ==
          std::vector{"AA"_ast_re, "AKs"_ast_re});
    CHECK(range.weighted_elems[0].weight == 90.0);
    CHECK(range.weighted_elems[1].elems == std::vector{"AQs"_ast_re});
    CHECK(range.weighted_elems[1].weight == 85.0);
  }

  SECTION("Grouped range")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "gtoplus" / "grouped_range.txt");
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

    CHECK(range.info.name == "grouped range");
    CHECK(range.info.type == gtoplus::parser::ast::entry_type::range);
    CHECK(range.info.nb_subentries == 4);
    REQUIRE(range.groups.size() == 4);
    CHECK(range.groups[0].name == "Group 1");
    CHECK(range.groups[0].type == gtoplus::parser::ast::entry_type::group);
    CHECK(range.groups[0].group_index == 0);
    CHECK(range.groups[0].nb_subentries == 0);
    CHECK(range.groups[1].name == "Group 2");
    CHECK(range.groups[1].type == gtoplus::parser::ast::entry_type::group);
    CHECK(range.groups[1].group_index == 1);
    CHECK(range.groups[1].nb_subentries == 0);
    CHECK(range.groups[2].name == "Group 3");
    CHECK(range.groups[2].type == gtoplus::parser::ast::entry_type::group);
    CHECK(range.groups[2].group_index == 2);
    CHECK(range.groups[2].nb_subentries == 0);
    CHECK(range.groups[3].name == "Group 9");
    CHECK(range.groups[3].type == gtoplus::parser::ast::entry_type::group);
    CHECK(range.groups[3].group_index == 8);
    CHECK(range.groups[3].nb_subentries == 0);
    REQUIRE(range.weighted_elems.size() == 1);
    CHECK(range.weighted_elems.front().elems == std::vector{"AA"_ast_re});
    CHECK(range.weighted_elems.front().weight == 100.0);
    REQUIRE(range.hand_info.size() == 169);
    CHECK(range.hand_info.front().hand ==
          prc::parser::ast::hand{prc::parser::ast::paired_hand{rank::ace}});
    auto const& group_ratios = range.hand_info.front().group_ratios;
    REQUIRE(group_ratios.size() == 4);
    CHECK(group_ratios[0].index == 0);
    CHECK(group_ratios[0].ratio == 1.0);
    CHECK(group_ratios[1].index == 1);
    CHECK(group_ratios[1].ratio == 1.0);
    CHECK(group_ratios[2].index == 2);
    CHECK(group_ratios[2].ratio == 1.0);
    CHECK(group_ratios[3].index == 8);
    CHECK(group_ratios[3].ratio == 1.0);

    auto b = std::all_of(
        range.hand_info.begin() + 1, range.hand_info.end(), [](auto& e) {
          return e.group_ratios.size() == 1 &&
                 e.group_ratios.front().index == 0 &&
                 e.group_ratios.front().ratio == 1.0;
        });
    CHECK(b);
  }
}
