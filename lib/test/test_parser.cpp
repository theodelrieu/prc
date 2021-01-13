#include <catch2/catch.hpp>

#include <iostream>
#include <set>

#include <prc/combo.hpp>
#include <prc/parser/api.hpp>
#include <prc/parser/ast.hpp>
#include <prc/parser/ast_adapted.hpp>
#include <prc/parser/config.hpp>

using namespace prc;
using namespace std::string_literals;
namespace x3 = boost::spirit::x3;

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
}

TEST_CASE("parser tests", "[parser]")
{
  SECTION("card")
  {
    SECTION("valid")
    {
      auto const input = "Ah,Kc,2d,9s"s;

      auto ctx = init_context(input, parser::card() % x3::lit(","));
      std::vector<parser::ast::card> cards;
      REQUIRE_NOTHROW(
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, cards));
      REQUIRE(cards.size() == 4);

      CHECK(cards[0].rank == prc::rank::ace);
      CHECK(cards[0].suit == prc::suit::heart);
      CHECK(cards[1].rank == prc::rank::king);
      CHECK(cards[1].suit == prc::suit::club);
      CHECK(cards[2].rank == prc::rank::two);
      CHECK(cards[2].suit == prc::suit::diamond);
      CHECK(cards[3].rank == prc::rank::nine);
      CHECK(cards[3].suit == prc::suit::spade);
    }

    SECTION("invalid suit")
    {
      auto const input = "Aq"s;
      auto ctx = init_context(input, parser::card());
      parser::ast::card card;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, card);
      CHECK_FALSE(r);
    }

    SECTION("invalid rank")
    {
      auto const input = "Sh"s;
      auto ctx = init_context(input, parser::card());
      parser::ast::card card;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, card);
      CHECK_FALSE(r);
    }
  }

  SECTION("combo")
  {
    SECTION("valid")
    {
      auto const input = "Ah2h,KcQd,2d2s,9s2h"s;

      auto ctx = init_context(input, parser::combo() % x3::lit(","));
      std::vector<parser::ast::combo> combos;
      REQUIRE_NOTHROW(
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combos));
      REQUIRE(combos.size() == 4);

      CHECK(combos[0].first_card.rank == prc::rank::ace);
      CHECK(combos[0].first_card.suit == prc::suit::heart);
      CHECK(combos[0].second_card.rank == prc::rank::two);
      CHECK(combos[0].second_card.suit == prc::suit::heart);

      CHECK(combos[1].first_card.rank == prc::rank::king);
      CHECK(combos[1].first_card.suit == prc::suit::club);
      CHECK(combos[1].second_card.rank == prc::rank::queen);
      CHECK(combos[1].second_card.suit == prc::suit::diamond);

      CHECK(combos[2].first_card.rank == prc::rank::two);
      CHECK(combos[2].first_card.suit == prc::suit::diamond);
      CHECK(combos[2].second_card.rank == prc::rank::two);
      CHECK(combos[2].second_card.suit == prc::suit::spade);

      CHECK(combos[3].first_card.rank == prc::rank::nine);
      CHECK(combos[3].first_card.suit == prc::suit::spade);
      CHECK(combos[3].second_card.rank == prc::rank::two);
      CHECK(combos[3].second_card.suit == prc::suit::heart);
    }

    SECTION("invalid first suit")
    {
      auto const input = "AxQh"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }

    SECTION("invalid first rank")
    {
      auto const input = "XhQs"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }

    SECTION("invalid second suit")
    {
      auto const input = "AhQx"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }

    SECTION("invalid second rank")
    {
      auto const input = "AhXs"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }

    SECTION("invalid rank order")
    {
      auto const input = "KhAs"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }

    SECTION("same cards")
    {
      auto const input = "KhKh"s;
      auto ctx = init_context(input, parser::combo());
      parser::ast::combo combo;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, combo);
      CHECK_FALSE(r);
    }
  }

  SECTION("unpaired hand")
  {
    SECTION("valid")
    {
      auto const input = "ATo,AKs,J7o,Q9s"s;

      auto ctx = init_context(input, parser::unpaired_hand() % x3::lit(","));
      std::vector<parser::ast::unpaired_hand> hands;
      REQUIRE_NOTHROW(
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, hands));
      REQUIRE(hands.size() == 4);

      CHECK(hands[0].first_rank == prc::rank::ace);
      CHECK(hands[0].second_rank == prc::rank::ten);
      CHECK(hands[0].suitedness == suitedness::offsuit);

      CHECK(hands[1].first_rank == prc::rank::ace);
      CHECK(hands[1].second_rank == prc::rank::king);
      CHECK(hands[1].suitedness == suitedness::suited);

      CHECK(hands[2].first_rank == prc::rank::jack);
      CHECK(hands[2].second_rank == prc::rank::seven);
      CHECK(hands[2].suitedness == suitedness::offsuit);

      CHECK(hands[3].first_rank == prc::rank::queen);
      CHECK(hands[3].second_rank == prc::rank::nine);
      CHECK(hands[3].suitedness == suitedness::suited);
    }

    SECTION("invalid")
    {
      auto const input = "AKa"s;
      auto ctx = init_context(input, parser::unpaired_hand());
      parser::ast::unpaired_hand hand;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, hand);
      CHECK_FALSE(r);
    }
  }

  SECTION("paired hand")
  {
    SECTION("valid")
    {
      auto const input = "AA,33,77,JJ"s;

      auto ctx = init_context(input, parser::paired_hand() % x3::lit(","));
      std::vector<parser::ast::paired_hand> hands;
      REQUIRE_NOTHROW(x3::phrase_parse(
          input.begin(), input.end(), ctx > x3::eoi, x3::space, hands));
      REQUIRE(hands.size() == 4);

      CHECK(hands[0].rank == prc::rank::ace);
      CHECK(hands[1].rank == prc::rank::three);
      CHECK(hands[2].rank == prc::rank::seven);
      CHECK(hands[3].rank == prc::rank::jack);
    }

    SECTION("invalid")
    {
      auto const input = "AKs"s;
      auto ctx = init_context(input, parser::paired_hand());
      parser::ast::paired_hand hand;
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, hand);
      CHECK_FALSE(r);
    }
  }

  SECTION("hand")
  {
    SECTION("valid")
    {
      auto const input = "AA,32o,77,KQs"s;

      auto ctx = init_context(input, parser::hand() % x3::lit(","));
      std::vector<parser::ast::hand> hands;
      REQUIRE_NOTHROW(x3::phrase_parse(
          input.begin(), input.end(), ctx > x3::eoi, x3::space, hands));
      REQUIRE(hands.size() == 4);

      CHECK(boost::get<parser::ast::paired_hand>(hands[0]).rank ==
            prc::rank::ace);

      CHECK(boost::get<parser::ast::unpaired_hand>(hands[1]).first_rank ==
            prc::rank::three);
      CHECK(boost::get<parser::ast::unpaired_hand>(hands[1]).second_rank ==
            prc::rank::two);
      CHECK(boost::get<parser::ast::unpaired_hand>(hands[1]).suitedness ==
            suitedness::offsuit);

      CHECK(boost::get<parser::ast::paired_hand>(hands[2]).rank ==
            prc::rank::seven);
      CHECK(boost::get<parser::ast::unpaired_hand>(hands[3]).first_rank ==
            prc::rank::king);
      CHECK(boost::get<parser::ast::unpaired_hand>(hands[3]).second_rank ==
            prc::rank::queen);
      CHECK(boost::get<parser::ast::unpaired_hand>(hands[3]).suitedness ==
            suitedness::suited);
    }

    SECTION("invalid")
    {
      auto const input = "AAs,KJo"s;
      std::vector<parser::ast::hand> hands;
      auto ctx = init_context(input, (parser::hand() % ',') >> x3::eoi);
      auto const r =
          x3::phrase_parse(input.begin(), input.end(), ctx, x3::space, hands);
      CHECK_FALSE(r);
    }
  }

  SECTION("hand range")
  {
    SECTION("valid")
    {
      auto const input = "22-66,KTo+,77+,98s+,J8o-JTo,AKs-87s"s;

      auto ctx = init_context(input, parser::hand_range() % x3::lit(","));
      std::vector<parser::ast::hand_range> hand_ranges;
      REQUIRE_NOTHROW(x3::phrase_parse(
          input.begin(), input.end(), ctx, x3::space, hand_ranges));
      REQUIRE(hand_ranges.size() == 6);

      CHECK((hand_ranges[0] ==
             parser::ast::hand_range{parser::ast::paired_hand{rank::two},
                                     parser::ast::paired_hand{rank::six}}));
      CHECK((hand_ranges[1] ==
             parser::ast::hand_range{
                 parser::ast::unpaired_hand{
                     rank::king, rank::ten, suitedness::offsuit},
                 parser::ast::unpaired_hand{
                     rank::king, rank::queen, suitedness::offsuit}}));
      CHECK((hand_ranges[2] ==
             parser::ast::hand_range{parser::ast::paired_hand{rank::seven},
                                     parser::ast::paired_hand{rank::ace}}));
      CHECK((hand_ranges[3] ==
             parser::ast::hand_range{
                 parser::ast::unpaired_hand{
                     rank::nine, rank::eight, suitedness::suited},
                 parser::ast::unpaired_hand{
                     rank::ace, rank::king, suitedness::suited}}));
      CHECK((hand_ranges[4] ==
             parser::ast::hand_range{
                 parser::ast::unpaired_hand{
                     rank::jack, rank::eight, suitedness::offsuit},
                 parser::ast::unpaired_hand{
                     rank::jack, rank::ten, suitedness::offsuit}}));
      CHECK((hand_ranges[5] ==
             parser::ast::hand_range{
                 parser::ast::unpaired_hand{
                     rank::ace, rank::king, suitedness::suited},
                 parser::ast::unpaired_hand{
                     rank::eight, rank::seven, suitedness::suited}}));
    }

    SECTION("invalid")
    {
      auto const input = {"AA-AA"s,
                          "33-33"s,
                          "AA+"s,
                          "KQ-AK"s,
                          "KTs-AKs"s,
                          "J8o-A8o"s,
                          "KTs-KQo"s,
                          "KK-JTo"s};
      for (auto const& elem : input)
      {
        parser::ast::hand_range hand_range;
        auto ctx = init_context(elem, parser::hand_range());
        auto const r = x3::phrase_parse(
            elem.begin(), elem.end(), ctx, x3::space, hand_range);
        CHECK_FALSE(r);
      }
    }
  }
}
