#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/locale.hpp>
#include <catch2/catch.hpp>

#include <prc/equilab/parser/api.hpp>
#include <prc/equilab/serialize.hpp>
#include <prc/folder.hpp>
#include <prc/range.hpp>

extern std::string testDataPath;

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;
using namespace prc;
using namespace prc::parser::ast::literals;

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
  std::ifstream ifs{p, std::ios::binary};
  std::string content(std::istreambuf_iterator<char>(ifs), {});
  auto utf8 = boost::locale::conv::between(content, "UTF8", "UTF16-LE");
  return boost::locale::conv::utf_to_utf<char32_t>(utf8);
}
}

using namespace std::string_literals;
TEST_CASE("equilab format tests", "[equilab]")
{
  std::vector const any_two{
      "22+"_ast_re,  "A2o+"_ast_re, "K2o+"_ast_re, "Q2o+"_ast_re, "J2o+"_ast_re,
      "T2o+"_ast_re, "92o+"_ast_re, "82o+"_ast_re, "72o+"_ast_re, "62o+"_ast_re,
      "52o+"_ast_re, "42o+"_ast_re, "32o"_ast_re,  "A2s+"_ast_re, "K2s+"_ast_re,
      "Q2s+"_ast_re, "J2s+"_ast_re, "T2s+"_ast_re, "92s+"_ast_re, "82s+"_ast_re,
      "72s+"_ast_re, "62s+"_ast_re, "52s+"_ast_re, "42s+"_ast_re, "32s"_ast_re};

  SECTION("pairs")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "pairs.hr");
    auto ctx = init_context(content, equilab::parser::file());

    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    REQUIRE(entries.size() == 1);
    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "pairs");
    REQUIRE(range.groups.size() == 1);
    auto& group = range.groups.front();
    CHECK_FALSE(group.info.has_value());
    REQUIRE(group.weighted_hands.size() == 1);
    auto& weighted_hands = group.weighted_hands.front();
    CHECK(weighted_hands.weight == 100.0);
    REQUIRE(weighted_hands.hands.size() == 1);

    CHECK(weighted_hands.hands.front() == "22+"_ast_re);
  }

  SECTION("1 group")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "1 group.hr");
    auto ctx = init_context(content, equilab::parser::file());

    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    REQUIRE(entries.size() == 1);
    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "1 group");
    REQUIRE(range.groups.size() == 2);
    auto& group = range.groups.front();
    REQUIRE(group.info.has_value());
    CHECK(group.info->index == 0);
    CHECK(group.info->parent_index == 0);
    CHECK(group.info->nesting_index == 0);
    CHECK(group.info->rgb == 0xffc0c0c0);
    CHECK(group.info->name.empty());

    REQUIRE(group.weighted_hands.size() == 1);
    auto& weighted_hands = group.weighted_hands.front();
    CHECK(weighted_hands.weight == 100.0);
    REQUIRE(weighted_hands.hands.size() == 1);

    CHECK(weighted_hands.hands.front() == "J7s"_ast_re);
  }

  SECTION("no group, multiple weights")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "multiple_weights.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);
    REQUIRE(entries.size() == 1);

    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "multiple weights");
    REQUIRE(range.groups.size() == 1);
    auto& group = range.groups.front();
    REQUIRE_FALSE(group.info.has_value());
    REQUIRE(group.weighted_hands.size() == 3);
    CHECK(group.weighted_hands[0].weight == 100.0);
    CHECK(group.weighted_hands[0].hands ==
          std::vector{"AA"_ast_re, "AKs"_ast_re});
    CHECK(group.weighted_hands[1].weight == 99.0);
    CHECK(group.weighted_hands[1].hands ==
          std::vector{"AQs"_ast_re, "KJs"_ast_re});
    CHECK(group.weighted_hands[2].weight == 96.0);
    CHECK(group.weighted_hands[2].hands ==
          std::vector{"A8s"_ast_re, "Q8s"_ast_re, "T8s"_ast_re});

    prc::range const abstract_range{range};
    std::vector<prc::range::weighted_elems> const expected = {
        {100.0, {"AA"_re, "AKs"_re}},
        {99.0, {"KJs"_re, "AQs"_re}},
        {96.0, {"T8s"_re, "Q8s"_re, "A8s"_re}}};
    CHECK(abstract_range.elems() == expected);

    prc::folder const f("/", entries);

    REQUIRE(f.entries().size() == 1);
    REQUIRE(
        boost::variant2::holds_alternative<prc::range>(f.entries().front()));
    CHECK(boost::variant2::get<prc::range>(f.entries().front()) ==
          abstract_range);
  }

  SECTION("random")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "random.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "random");
    REQUIRE(range.groups.size() == 1);
    auto& group = range.groups.front();
    REQUIRE_FALSE(group.info.has_value());
    REQUIRE(group.weighted_hands.size() == 1);
    CHECK(group.weighted_hands.front().hands == any_two);
  }

  SECTION("2 groups")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "2_groups.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "2 groups");
    REQUIRE(range.groups.size() == 3);
    auto& parent_group = range.groups[0];
    REQUIRE(parent_group.info.has_value());
    REQUIRE(parent_group.weighted_hands.size() == 1);

    CHECK(parent_group.weighted_hands[0].weight == 100.0);
    CHECK(parent_group.weighted_hands[0].hands == any_two);
    CHECK(parent_group.info->index == 0);
    CHECK(parent_group.info->parent_index == 0);
    CHECK(parent_group.info->nesting_index == 0);
    CHECK(parent_group.info->rgb == 0xffc0c0c0);
    CHECK(parent_group.info->name.empty());

    auto& raise_group = range.groups[1];
    REQUIRE(raise_group.info.has_value());
    REQUIRE(raise_group.weighted_hands.size() == 2);

    CHECK(raise_group.info->index == 1);
    CHECK(raise_group.info->parent_index == 0);
    CHECK(raise_group.info->nesting_index == 0);
    CHECK(raise_group.info->name == "Raise");
    CHECK(raise_group.info->rgb == 0xffe48073);
    CHECK(raise_group.weighted_hands[0].weight == 100.0);
    CHECK(raise_group.weighted_hands[0].hands ==
          std::vector{"22+"_ast_re, "A2s+"_ast_re});
    CHECK(raise_group.weighted_hands[1].weight == 95.0);
    CHECK(raise_group.weighted_hands[1].hands == std::vector{"A2o+"_ast_re});

    auto& call_group = range.groups[2];
    REQUIRE(call_group.info.has_value());
    REQUIRE(call_group.weighted_hands.size() == 2);

    CHECK(call_group.info->index == 2);
    CHECK(call_group.info->parent_index == 0);
    CHECK(call_group.info->nesting_index == 0);
    CHECK(call_group.info->name == "Call");
    CHECK(call_group.info->rgb == 0xff80ff00);
    CHECK(call_group.weighted_hands[0].weight == 100.0);
    CHECK(call_group.weighted_hands[0].hands == std::vector{"K2s+"_ast_re});
    CHECK(call_group.weighted_hands[1].weight == 95.0);
    CHECK(call_group.weighted_hands[1].hands == std::vector{"K2o+"_ast_re});
  }

  SECTION("unicode")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "unicode.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == u8"\xed\x95\x9c\xea\xb8\x80");
    REQUIRE(range.groups.size() == 1);
    auto& group = range.groups.front();
    REQUIRE(group.weighted_hands.size() == 1);
    CHECK(group.weighted_hands.front().weight == 100.0);
    CHECK(group.weighted_hands.front().hands == std::vector{"AA"_ast_re});
  }

  SECTION("folders")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "folders.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    REQUIRE(entries.size() == 3);
    auto& folder1 = boost::get<equilab::parser::ast::folder>(entries[0]);
    auto& subfolder1 = boost::get<equilab::parser::ast::folder>(entries[1]);
    auto& folder2 = boost::get<equilab::parser::ast::folder>(entries[2]);

    CHECK(folder1.depth == 1);
    CHECK(folder1.name == "folder1");
    CHECK(subfolder1.depth == 2);
    CHECK(subfolder1.name == "subfolder1");
    CHECK(folder2.depth == 1);
    CHECK(folder2.name == "folder2");
  }

  SECTION("note")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "note.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);
    REQUIRE(entries.size() == 1);
    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "random");
    CHECK(range.note == "some note"s);
  }

  SECTION("nested groups")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "nested.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    auto& range = boost::get<equilab::parser::ast::range>(entries.front());
    CHECK(range.depth == 1);
    CHECK(range.name == "nested");
    REQUIRE(range.groups.size() == 5);
    auto& parent_group = range.groups[0];
    REQUIRE(parent_group.info.has_value());
    REQUIRE(parent_group.weighted_hands.size() == 1);

    CHECK(parent_group.weighted_hands[0].weight == 96.0);
    CHECK(parent_group.weighted_hands[0].hands == any_two);
    CHECK(parent_group.info->index == 0);
    CHECK(parent_group.info->parent_index == 0);
    CHECK(parent_group.info->nesting_index == 0);
    CHECK(parent_group.info->rgb == 0xffc0c0c0);
    CHECK(parent_group.info->name.empty());

    auto& raise_group = range.groups[1];
    REQUIRE(raise_group.info.has_value());
    REQUIRE(raise_group.weighted_hands.size() == 1);
    CHECK(raise_group.info->index == 1);
    CHECK(raise_group.info->parent_index == 0);
    CHECK(raise_group.info->nesting_index == 0);
    CHECK(raise_group.info->name == "Raise");
    CHECK(raise_group.info->rgb == 0xffe48073);
    CHECK(raise_group.weighted_hands[0].weight == 100.0);
    CHECK(raise_group.weighted_hands[0].hands ==
          std::vector{"AA"_ast_re, "AKs"_ast_re});

    auto& call_group = range.groups[2];
    REQUIRE(call_group.info.has_value());
    REQUIRE(call_group.weighted_hands.size() == 1);

    CHECK(call_group.info->index == 2);
    CHECK(call_group.info->parent_index == 0);
    CHECK(call_group.info->nesting_index == 0);
    CHECK(call_group.info->name == "Call");
    CHECK(call_group.info->rgb == 0xff80ff00);
    CHECK(call_group.weighted_hands[0].weight == 100.0);
    CHECK(call_group.weighted_hands[0].hands ==
          std::vector{"AQs"_ast_re, "AKo"_ast_re});

    auto& call_vs_3bet_group = range.groups[3];
    REQUIRE(call_vs_3bet_group.info.has_value());
    REQUIRE(call_vs_3bet_group.weighted_hands.size() == 1);

    CHECK(call_vs_3bet_group.info->index == 3);
    CHECK(call_vs_3bet_group.info->parent_index == 1);
    CHECK(call_vs_3bet_group.info->nesting_index == 1);
    CHECK(call_vs_3bet_group.info->name == "Call vs 3bet");
    CHECK(call_vs_3bet_group.info->rgb == 0xff80ff00);
    CHECK(call_vs_3bet_group.weighted_hands[0].weight == 100.0);
    CHECK(call_vs_3bet_group.weighted_hands[0].hands ==
          std::vector{"AA"_ast_re});

    auto& fold_vs_3bet_group = range.groups[4];
    REQUIRE(fold_vs_3bet_group.info.has_value());
    REQUIRE(fold_vs_3bet_group.weighted_hands.size() == 1);

    CHECK(fold_vs_3bet_group.info->index == 4);
    CHECK(fold_vs_3bet_group.info->parent_index == 1);
    CHECK(fold_vs_3bet_group.info->nesting_index == 1);
    CHECK(fold_vs_3bet_group.info->name == "Fold vs 3bet");
    CHECK(fold_vs_3bet_group.info->rgb == 0xff0080ff);
    CHECK(fold_vs_3bet_group.weighted_hands[0].weight == 100.0);
    CHECK(fold_vs_3bet_group.weighted_hands[0].hands ==
          std::vector{"AKs"_ast_re});
  }

  SECTION("folders and ranges")
  {
    auto const content =
        read_all(fs::path{testDataPath} / "equilab" / "folders_and_ranges.hr");
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto const e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto const r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    prc::folder const f("/", entries);

    auto const& elems = f.entries();
    REQUIRE(elems.size() == 3);
    REQUIRE(boost::variant2::holds_alternative<folder>(elems[0]));
    REQUIRE(boost::variant2::holds_alternative<folder>(elems[1]));
    REQUIRE(boost::variant2::holds_alternative<prc::range>(elems[2]));

    auto& folder1 = boost::variant2::get<folder>(elems[0]);
    auto& folder2 = boost::variant2::get<folder>(elems[1]);
    auto& nested = boost::variant2::get<range>(elems[2]);

    CHECK(folder1.name() == "folder1");
    CHECK(folder2.name() == "folder2");
    CHECK(nested.name() == "nested");

    REQUIRE(folder1.entries().size() == 2);
    REQUIRE(boost::variant2::holds_alternative<prc::folder>(
        folder1.entries().front()));
    REQUIRE(boost::variant2::holds_alternative<prc::range>(
        folder1.entries().back()));

    std::vector<prc::range::weighted_elems> expected = {
        {100.0, {"AA"_re, "AKs"_re}},
        {99.0, {"KJs"_re, "AQs"_re}},
        {96.0, {"T8s"_re, "Q8s"_re, "A8s"_re}}};
    CHECK(boost::variant2::get<prc::range>(folder1.entries().back()).elems() ==
          expected);

    auto& subfolder1 = boost::variant2::get<folder>(folder1.entries().front());
    CHECK(subfolder1.name() == "subfolder1");
    REQUIRE(subfolder1.entries().size() == 1);
    REQUIRE(boost::variant2::holds_alternative<prc::range>(
        subfolder1.entries().front()));
    CHECK(boost::variant2::get<prc::range>(subfolder1.entries().front())
              .elems() ==
          std::vector{range::weighted_elems{96.0, prc::any_two()}});

    CHECK(folder2.name() == "folder2");
    REQUIRE(folder2.entries().size() == 1);
    REQUIRE(boost::variant2::holds_alternative<prc::range>(
        folder2.entries().front()));
    auto& folder2_base_range =
        boost::variant2::get<prc::range>(folder2.entries().front());
    CHECK(folder2_base_range.elems() ==
          std::vector<range::weighted_elems>{{100.0, {"K9s"_re}},
                                             {80.0, {"AQs"_re}}});

    REQUIRE(folder2_base_range.subranges().size() == 1);
    auto& folder2_subrange = folder2_base_range.subranges().front();
    CHECK(folder2_subrange.name() == "Group 1");
    CHECK(folder2_subrange.rgb() == 0xffe48073);
    CHECK(folder2_subrange.elems() ==
          std::vector<range::weighted_elems>{{100.0, {"K9s"_re, "AQs"_re}}});
  }
}

TEST_CASE("serialization tests", "[equilab]")
{
  // nested not supported
  auto const files = {"1 group.hr"s,
                      "2_groups.hr"s,
                      "folders.hr"s,
                      "folders_and_ranges.hr"s,
                      "multiple_weights.hr"s,
                      "pairs.hr"s,
                      "nested.hr"s,
                      "random.hr"s,
                      "unicode.hr"s};
  for (auto const& f : files)
  {
    auto const content = read_all(fs::path{testDataPath} / "equilab" / f);
    auto ctx = init_context(content, equilab::parser::file());
    auto b = content.begin();
    auto e = content.end();
    std::vector<equilab::parser::ast::entry> entries;
    auto r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    prc::folder folder{"/", entries};
    auto const serialized = equilab::serialize(folder);
    auto const utf8 =
        boost::locale::conv::between(serialized, "UTF8", "UTF16-LE");
    std::cout << utf8 << std::endl;
    auto const utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8);
    b = utf32.begin();
    e = utf32.end();
    entries.clear();
    r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
    REQUIRE(r);
    REQUIRE(b == e);

    CHECK(folder == prc::folder{"/", entries});
  }
}
