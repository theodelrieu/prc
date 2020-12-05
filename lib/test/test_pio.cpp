#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/locale.hpp>
#include <catch2/catch.hpp>

#include <prc/combo.hpp>
#include <prc/pio/parser/api.hpp>
#include <prc/range.hpp>
#include <prc/range_elem.hpp>

extern std::string testDataPath;

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;
using namespace prc::literals;
using namespace prc;

namespace
{
template <typename Parser>
auto init_context(std::u32string const& input,
                  Parser p,
                  std::ostream& error_stream = std::cerr)
{
  x3::error_handler<std::u32string::const_iterator> error_handler(
      input.begin(), input.end(), error_stream);

  return x3::with<x3::error_handler_tag>(std::move(error_handler))[p];
}

std::u32string read_all(fs::path const& p)
{
  if (!fs::exists(p))
  {
    throw std::runtime_error("No such path: " + p.string());
  }
  std::ifstream ifs{p};
  std::string content(std::istreambuf_iterator<char>(ifs), {});
  return boost::locale::conv::utf_to_utf<char32_t>(content);
}

std::vector<prc::combo> weights_to_combos(
    std::vector<prc::combo> const& any_two, std::vector<double> const& weights)
{
  std::vector<prc::combo> ret;

  for (auto i = 0; i < any_two.size(); ++i)
  {
    if (weights[i] != 0.0)
      ret.push_back(any_two[i]);
  }
  return ret;
}
}

TEST_CASE("pio format tests", "[pio]")
{
  SECTION("Pairs")
  {
    auto const content = read_all(fs::path{testDataPath} / "pio" / "pairs.txt");
    auto ctx = init_context(content, pio::parser::range());

    pio::parser::ast::range range;
    auto const r = x3::phrase_parse(
        content.begin(), content.end(), ctx, x3::unicode::space, range);
    REQUIRE(r);

    CHECK(range.base_range.name == "PreflopCharts");
    REQUIRE(range.subranges.size() == 4);
    CHECK(range.subranges[0].included);
    CHECK(range.subranges[0].name == "Range 1");
    CHECK(range.subranges[0].rgb == 0xFFE9967A);

    auto const& weights = range.subranges.front().weights;
    CHECK(weights.size() == 1326);

    prc::range const abstract_range{range};
    CHECK(abstract_range.rgb() == 0);
    auto const& elems = abstract_range.elems();
    REQUIRE(elems.size() == 1);
    CHECK(elems.front().weight == 100.0);
    CHECK(elems.front().elems == any_two());

    REQUIRE(abstract_range.subranges().size() == 1);
    auto const& raise_subrange = abstract_range.subranges().front();
    CHECK(raise_subrange.rgb() == 0xFFE9967A);
    CHECK(raise_subrange.name() == "Range 1");
    auto const& sub_elems = raise_subrange.elems();
    REQUIRE(sub_elems.size() == 1);
    CHECK(sub_elems.front().weight == 100.0);
    CHECK(sub_elems.front().elems == std::vector{"22+"_re});
  }
}
