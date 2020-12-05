#pragma once

// #define BOOST_SPIRIT_X3_DEBUG
#include <boost/spirit/home/x3.hpp>
#include <boost/variant.hpp>

#include <prc/parser/api.hpp>
#include <prc/parser/as_type.hpp>
#include <prc/parser/ast.hpp>
#include <prc/parser/ast_adapted.hpp>
#include <prc/parser/config.hpp>

namespace prc
{
namespace parser
{
namespace
{
auto const add_to_rank = [](prc::rank r, int n) {
  return static_cast<prc::rank>(static_cast<int>(r) + n);
};
}

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

  template <typename Iterator, typename Context>
  void on_success(Iterator first,
                  Iterator last,
                  ast::combo const& c,
                  Context const& ctx)
  {
    if (c.first_card.rank < c.second_card.rank ||
        (c.first_card.rank == c.second_card.rank &&
         c.first_card.suit == c.second_card.suit))
    {
      throw x3::expectation_failure<Iterator>(first, "combo");
    }
  }

  template <typename Iterator, typename Context>
  void on_success(Iterator first,
                  Iterator last,
                  ast::unpaired_hand const& h,
                  Context const& ctx)
  {
    if (h.first_rank <= h.second_rank)
      throw x3::expectation_failure<Iterator>(first, "unpaired hand");
  }

  template <typename Iterator, typename Context>
  void on_success(Iterator first,
                  Iterator last,
                  ast::hand_range& h,
                  Context const& ctx)
  {
    if (h.from.which() != h.to.which())
      throw x3::expectation_failure<Iterator>(first, "hand range");

    if (h.from == h.to)
      throw x3::expectation_failure<Iterator>(first, "hand range");
    if (auto p = boost::get<ast::unpaired_hand>(&h.to))
    {
      auto& f = boost::get<ast::unpaired_hand>(h.from);
      if (p->suitedness != f.suitedness)
        throw x3::expectation_failure<Iterator>(first, "hand range");
      if (p->first_rank != f.first_rank)
      {
        // KQo-AKo is valid, whereas KJo-AKo is not
        if ((add_to_rank(p->second_rank, 1) != p->first_rank) ||
            add_to_rank(f.second_rank, 1) != f.first_rank)
        {
          throw x3::expectation_failure<Iterator>(first, "hand range");
        }
      }
    }
  }
};

card_type const _card = "card";
combo_type const _combo = "combo";
unpaired_hand_type const _unpaired_hand = "unpaired hand";
hand_type const _hand = "hand";
hand_range_type const _hand_range = "hand range";
range_elem_type const _range_elem = "range elem";

auto const char_to_suitedness = [](auto& ctx) {
  if (_attr(ctx) == 'o')
    _val(ctx) = suitedness::offsuit;
  else
    _val(ctx) = suitedness::suited;
};

auto const set_hand_range_from = [](auto& ctx) { _val(ctx).from = _attr(ctx); };
auto const set_hand_range_to = [](auto& ctx) { _val(ctx).to = _attr(ctx); };
auto const fill_hand_range = [](auto& ctx) {
  auto& h = _val(ctx);
  if (auto p = boost::get<ast::paired_hand>(&h.from))
  {
    // AA+ is invalid
    if (p->rank == rank::ace)
    {
      _pass(ctx) = false;
      return;
    }
    h.to = ast::paired_hand{rank::ace};
  }
  else
  {
    auto& up = boost::get<ast::unpaired_hand>(h.from);
    if (up.second_rank == add_to_rank(up.first_rank, -1))
      h.to = ast::unpaired_hand{rank::ace, rank::king, up.suitedness};
    else
    {
      h.to = ast::unpaired_hand{
          up.first_rank, add_to_rank(up.first_rank, -1), up.suitedness};
    }
  }
};

auto const _card_def = rank() >> suit();
auto const _combo_def = _card > _card;

auto const _unpaired_hand_def =
    rank() >> rank() >>
    as<prc::suitedness>[x3::unicode::char_(U"os")[char_to_suitedness]];

auto const _hand_def = paired_hand() | unpaired_hand();

auto const _hand_range_def =
    as<ast::hand_range>[hand()[set_hand_range_from] >>
                        (x3::unicode::lit(U"+")[fill_hand_range] |
                         (x3::unicode::lit(U"-") > hand()[set_hand_range_to]))];

auto const _range_elem_def = combo() | hand_range() | hand();

BOOST_SPIRIT_DEFINE(
    _card, _combo, _unpaired_hand, _hand, _hand_range, _range_elem);

struct card_class : error_handler
{
};

struct combo_class : error_handler
{
};

struct unpaired_hand_class : error_handler
{
};

struct hand_range_class : error_handler
{
};

struct range_elem_class : error_handler
{
};
}
}
