#include <catch2/catch.hpp>

#include <prc/range.hpp>
#include <prc/range_elem.hpp>

namespace
{
template <typename T>
std::vector<T> sorted_vector(std::initializer_list<T> l)
{
  std::vector<T> ret(l.begin(), l.end());
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::vector<prc::range_elem> remove_elems(
    std::vector<prc::range_elem> const& parent,
    std::vector<prc::range_elem> const& to_remove)
{
  using namespace prc::literals;
  auto ret = parent;
  ret.erase(std::remove_if(ret.begin(), ret.end(), [&](auto& e) {
    return std::find(to_remove.begin(), to_remove.end(), e) != to_remove.end();
  }));
  std::sort(ret.begin(), ret.end());
  return ret;
}
}

TEST_CASE("expand/reduce tests", "[combos]")
{
  using namespace prc::literals;

  SECTION("expand")
  {
    SECTION("paired_hand")
    {
      auto const expected = sorted_vector(
          {"AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c, "AcAs"_c});

      auto const expanded = prc::expand_combos("AA"_re);
      CHECK(expanded == expected);
    }

    SECTION("combo")
    {
      auto const expanded = prc::expand_combos("AhAd"_re);
      CHECK(expanded == std::vector{"AhAd"_c});
    }

    SECTION("offsuit unpaired_hand")
    {
      auto const expanded = prc::expand_combos("AKo"_re);
      CHECK(expanded == sorted_vector({"AhKd"_c,
                                       "AsKd"_c,
                                       "AcKd"_c,
                                       "AdKh"_c,
                                       "AsKh"_c,
                                       "AcKh"_c,
                                       "AdKs"_c,
                                       "AhKs"_c,
                                       "AcKs"_c,
                                       "AdKc"_c,
                                       "AhKc"_c,
                                       "AsKc"_c}));
    }

    SECTION("suited unpaired_hand")
    {
      auto const expanded = prc::expand_combos("AKs"_re);
      CHECK(expanded ==
            sorted_vector({"AhKh"_c, "AsKs"_c, "AcKc"_c, "AdKd"_c}));
    }

    SECTION("hand range")
    {
      auto expanded = prc::expand_combos("AA-KK"_re);
      CHECK(expanded == sorted_vector({"AhAd"_c,
                                       "AhAs"_c,
                                       "AhAc"_c,
                                       "AdAs"_c,
                                       "AdAc"_c,
                                       "AcAs"_c,
                                       "KhKd"_c,
                                       "KhKs"_c,
                                       "KhKc"_c,
                                       "KdKs"_c,
                                       "KdKc"_c,
                                       "KcKs"_c}));
      expanded = prc::expand_combos("72s+"_re);
      CHECK(expanded ==
            sorted_vector({"7h2h"_c, "7s2s"_c, "7c2c"_c, "7d2d"_c, "7h3h"_c,
                           "7s3s"_c, "7c3c"_c, "7d3d"_c, "7h4h"_c, "7s4s"_c,
                           "7c4c"_c, "7d4d"_c, "7h5h"_c, "7s5s"_c, "7c5c"_c,
                           "7d5d"_c, "7h6h"_c, "7s6s"_c, "7c6c"_c, "7d6d"_c}));
    }
    SECTION("multiple combinations")
    {
      auto const expanded = prc::expand_combos(
          std::vector{"AKs"_re, "AKo"_re, "AhKs"_re, "AA"_re, "AA-KK"_re});
      CHECK(expanded ==
            sorted_vector({"AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c,
                           "AcAs"_c, "KhKd"_c, "KhKs"_c, "KhKc"_c, "KdKs"_c,
                           "KdKc"_c, "KcKs"_c, "AhKh"_c, "AsKs"_c, "AcKc"_c,
                           "AdKd"_c, "AhKd"_c, "AsKd"_c, "AcKd"_c, "AdKh"_c,
                           "AsKh"_c, "AcKh"_c, "AdKs"_c, "AhKs"_c, "AcKs"_c,
                           "AdKc"_c, "AhKc"_c, "AsKc"_c}));
    }
  }

  SECTION("reduce")
  {
    SECTION("paired hand")
    {
      auto reduced = prc::reduce_combos(std::vector{
          "AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c, "AcAs"_c});
      CHECK(reduced == std::vector{"AA"_re});

      reduced = prc::reduce_combos(
          std::vector{"AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c});
      CHECK(reduced ==
            sorted_vector(
                {"AhAd"_re, "AhAs"_re, "AhAc"_re, "AdAs"_re, "AdAc"_re}));

      reduced = prc::reduce_combos(std::vector{"AhAd"_c,
                                               "AhAs"_c,
                                               "AhAc"_c,
                                               "AdAs"_c,
                                               "AdAc"_c,
                                               "AcAs"_c,
                                               "QhQs"_c});
      CHECK(reduced == sorted_vector({"AA"_re, "QhQs"_re}));

      reduced = prc::reduce_combos(std::vector{"AhAd"_c,
                                               "AhAs"_c,
                                               "AhAc"_c,
                                               "AdAs"_c,
                                               "AdAc"_c,
                                               "AcAs"_c,
                                               "KhKd"_c,
                                               "KhKs"_c,
                                               "KhKc"_c,
                                               "KdKs"_c,
                                               "KdKc"_c,
                                               "KcKs"_c});
      CHECK(reduced == sorted_vector({"AA-KK"_re}));

      reduced = prc::reduce_combos(std::vector{"AhAd"_c,
                                               "AhAs"_c,
                                               "AhAc"_c,
                                               "AdAs"_c,
                                               "AdAc"_c,
                                               "AcAs"_c,
                                               "KhKd"_c,
                                               "KhKs"_c,
                                               "KhKc"_c,
                                               "KdKs"_c,
                                               "KdKc"_c,
                                               "KcKs"_c,
                                               "QhQd"_c,
                                               "QhQs"_c,
                                               "QhQc"_c,
                                               "QdQs"_c,
                                               "QdQc"_c,
                                               "QcQs"_c});
      CHECK(reduced == sorted_vector({"AA-QQ"_re}));

      reduced = prc::reduce_combos(std::vector{
          "AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c, "AcAs"_c,
          "KhKd"_c, "KhKs"_c, "KhKc"_c, "KdKs"_c, "KdKc"_c, "KcKs"_c,
          "QhQd"_c, "QhQs"_c, "QhQc"_c, "QdQs"_c, "QdQc"_c, "QcQs"_c,
          "7h7d"_c, "7h7s"_c, "7h7c"_c, "7d7s"_c, "7d7c"_c, "7c7s"_c});
      CHECK(reduced == sorted_vector({"AA-QQ"_re, "77"_re}));

      reduced = prc::reduce_combos(std::vector{
          "AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c, "AcAs"_c, "KhKd"_c,
          "KhKs"_c, "KhKc"_c, "KdKs"_c, "KdKc"_c, "KcKs"_c, "QhQd"_c, "QhQs"_c,
          "QhQc"_c, "QdQs"_c, "QdQc"_c, "QcQs"_c, "7h7d"_c, "7h7s"_c, "7h7c"_c,
          "7d7s"_c, "7d7c"_c, "7c7s"_c, "6h6d"_c, "6h6s"_c, "6h6c"_c, "6d6s"_c,
          "6d6c"_c, "6c6s"_c, "5h5d"_c, "5h5s"_c, "5h5c"_c, "5d5s"_c, "5d5c"_c,
          "5c5s"_c, "4h4d"_c, "4h4s"_c, "4h4c"_c, "4d4s"_c, "4d4c"_c, "4c4s"_c,
          "2h2d"_c, "2h2s"_c, "2h2c"_c, "2d2s"_c, "2d2c"_c, "2c2s"_c});
      CHECK(reduced == sorted_vector({"AA-QQ"_re, "77-44"_re, "22"_re}));
    }

    SECTION("suited unpaired hand")
    {
      auto reduced = prc::reduce_combos(
          std::vector{"AhKh"_c, "AsKs"_c, "AcKc"_c, "AdKd"_c});
      CHECK(reduced == std::vector{"AKs"_re});

      reduced = prc::reduce_combos(std::vector{"AhKh"_c, "AsKs"_c, "AcKc"_c});
      CHECK(reduced == sorted_vector({"AhKh"_re, "AsKs"_re, "AcKc"_re}));

      reduced = prc::reduce_combos(std::vector{"AhKh"_c,
                                               "AsKs"_c,
                                               "AcKc"_c,
                                               "AdKd"_c,
                                               "AhQh"_c,
                                               "AdQd"_c,
                                               "AsQs"_c,
                                               "AcQc"_c,
                                               "AhJh"_c,
                                               "AdJd"_c,
                                               "AsJs"_c,
                                               "AcJc"_c});
      CHECK(reduced == std::vector{"AKs-AJs"_re});

      reduced = prc::reduce_combos(std::vector{
          "9h6h"_c, "9s6s"_c, "9c6c"_c, "9d6d"_c, "9h5h"_c, "9d5d"_c, "9s5s"_c,
          "9c5c"_c, "9h4h"_c, "9d4d"_c, "9s4s"_c, "9c4c"_c, "KhQh"_c, "Th6h"_c,
          "Ts6s"_c, "Tc6c"_c, "Td6d"_c, "Th5h"_c, "Td5d"_c, "Ts5s"_c, "Tc5c"_c,
          "Th4h"_c, "Td4d"_c, "Ts4s"_c, "Tc4c"_c,
      });
      CHECK(reduced == sorted_vector({"96s-94s"_re, "KhQh"_re, "T4s-T6s"_re}));

      reduced = prc::reduce_combos(std::vector{"AhKh"_c,
                                               "AsKs"_c,
                                               "AcKc"_c,
                                               "AdKd"_c,
                                               "KhQh"_c,
                                               "KdQd"_c,
                                               "KsQs"_c,
                                               "KcQc"_c,
                                               "QhJh"_c,
                                               "QdJd"_c,
                                               "QsJs"_c,
                                               "QcJc"_c});
      CHECK(reduced == std::vector{"AKs-QJs"_re});
    }

    SECTION("offsuit unpaired hand")
    {
      auto reduced = prc::reduce_combos(std::vector{"AhKs"_c,
                                                    "AhKc"_c,
                                                    "AhKd"_c,
                                                    "AdKc"_c,
                                                    "AdKh"_c,
                                                    "AdKs"_c,
                                                    "AsKh"_c,
                                                    "AsKd"_c,
                                                    "AsKc"_c,
                                                    "AcKh"_c,
                                                    "AcKd"_c,
                                                    "AcKs"_c});
      CHECK(reduced == std::vector{"AKo"_re});

      reduced = prc::reduce_combos(std::vector{"AhKs"_c, "AsKh"_c, "AcKh"_c});
      CHECK(reduced == sorted_vector({"AhKs"_re, "AsKh"_re, "AcKh"_re}));

      reduced = prc::reduce_combos(std::vector{
          "AhKs"_c, "AhKc"_c, "AhKd"_c, "AdKc"_c, "AdKh"_c, "AdKs"_c,
          "AsKh"_c, "AsKd"_c, "AsKc"_c, "AcKh"_c, "AcKd"_c, "AcKs"_c,
          "AhQs"_c, "AhQc"_c, "AhQd"_c, "AdQc"_c, "AdQh"_c, "AdQs"_c,
          "AsQh"_c, "AsQd"_c, "AsQc"_c, "AcQh"_c, "AcQd"_c, "AcQs"_c,
          "AhJs"_c, "AhJc"_c, "AhJd"_c, "AdJc"_c, "AdJh"_c, "AdJs"_c,
          "AsJh"_c, "AsJd"_c, "AsJc"_c, "AcJh"_c, "AcJd"_c, "AcJs"_c});
      CHECK(reduced == std::vector{"AKo-AJo"_re});

      reduced = prc::reduce_combos(std::vector{
          "9h6s"_c, "9h6c"_c, "9h6d"_c, "9d6c"_c, "9d6h"_c, "9d6s"_c, "9s6h"_c,
          "9s6d"_c, "9s6c"_c, "9c6h"_c, "9c6d"_c, "9c6s"_c,

          "9h5s"_c, "9h5c"_c, "9h5d"_c, "9d5c"_c, "9d5h"_c, "9d5s"_c, "9s5h"_c,
          "9s5d"_c, "9s5c"_c, "9c5h"_c, "9c5d"_c, "9c5s"_c,

          "9h4s"_c, "9h4c"_c, "9h4d"_c, "9d4c"_c, "9d4h"_c, "9d4s"_c, "9s4h"_c,
          "9s4d"_c, "9s4c"_c, "9c4h"_c, "9c4d"_c, "9c4s"_c, "KhQs"_c,

          "Th6s"_c, "Th6c"_c, "Th6d"_c, "Td6c"_c, "Td6h"_c, "Td6s"_c, "Ts6h"_c,
          "Ts6d"_c, "Ts6c"_c, "Tc6h"_c, "Tc6d"_c, "Tc6s"_c,

          "Th5s"_c, "Th5c"_c, "Th5d"_c, "Td5c"_c, "Td5h"_c, "Td5s"_c, "Ts5h"_c,
          "Ts5d"_c, "Ts5c"_c, "Tc5h"_c, "Tc5d"_c, "Tc5s"_c,

          "Th4s"_c, "Th4c"_c, "Th4d"_c, "Td4c"_c, "Td4h"_c, "Td4s"_c, "Ts4h"_c,
          "Ts4d"_c, "Ts4c"_c, "Tc4h"_c, "Tc4d"_c, "Tc4s"_c,
      });

      CHECK(reduced == sorted_vector({"96o-94o"_re, "KhQs"_re, "T4o-T6o"_re}));

      reduced = prc::reduce_combos(std::vector{
          "AhKs"_c, "AhKc"_c, "AhKd"_c, "AdKc"_c, "AdKh"_c, "AdKs"_c,
          "AsKh"_c, "AsKd"_c, "AsKc"_c, "AcKh"_c, "AcKd"_c, "AcKs"_c,

          "KhQs"_c, "KhQc"_c, "KhQd"_c, "KdQc"_c, "KdQh"_c, "KdQs"_c,
          "KsQh"_c, "KsQd"_c, "KsQc"_c, "KcQh"_c, "KcQd"_c, "KcQs"_c,

          "QhJs"_c, "QhJc"_c, "QhJd"_c, "QdJc"_c, "QdJh"_c, "QdJs"_c,
          "QsJh"_c, "QsJd"_c, "QsJc"_c, "QcJh"_c, "QcJd"_c, "QcJs"_c});
      CHECK(reduced == std::vector{"AKo-QJo"_re});
    }

    SECTION("combining everything")
    {
      auto reduced = prc::reduce_combos(std::vector{
          "AhKs"_c, "AhKc"_c, "AhKd"_c, "AdKc"_c, "AdKh"_c, "AdKs"_c, "AsKh"_c,
          "AsKd"_c, "AsKc"_c, "AcKh"_c, "AcKd"_c, "AcKs"_c,

          "KhQs"_c, "KhQc"_c, "KhQd"_c, "KdQc"_c, "KdQh"_c, "KdQs"_c, "KsQh"_c,
          "KsQd"_c, "KsQc"_c, "KcQh"_c, "KcQd"_c, "KcQs"_c,

          "QhJs"_c, "QhJc"_c, "QhJd"_c, "QdJc"_c, "QdJh"_c, "QdJs"_c, "QsJh"_c,
          "QsJd"_c, "QsJc"_c, "QcJh"_c, "QcJd"_c, "QcJs"_c,

          "9h6h"_c, "9s6s"_c, "9c6c"_c, "9d6d"_c, "9h5h"_c, "9d5d"_c, "9s5s"_c,
          "9c5c"_c, "9h4h"_c, "9d4d"_c, "9s4s"_c, "9c4c"_c, "Th6h"_c, "Ts6s"_c,
          "Tc6c"_c, "Td6d"_c, "Th5h"_c, "Td5d"_c, "Ts5s"_c, "Tc5c"_c, "Th4h"_c,
          "Td4d"_c, "Ts4s"_c, "Tc4c"_c, "9h6s"_c, "9h6c"_c, "9h6d"_c, "9d6c"_c,
          "9d6h"_c, "9d6s"_c, "9s6h"_c, "9s6d"_c, "9s6c"_c, "9c6h"_c, "9c6d"_c,
          "9c6s"_c,

          "9h5s"_c, "9h5c"_c, "9h5d"_c, "9d5c"_c, "9d5h"_c, "9d5s"_c, "9s5h"_c,
          "9s5d"_c, "9s5c"_c, "9c5h"_c, "9c5d"_c, "9c5s"_c,

          "9h4s"_c, "9h4c"_c, "9h4d"_c, "9d4c"_c, "9d4h"_c, "9d4s"_c, "9s4h"_c,
          "9s4d"_c, "9s4c"_c, "9c4h"_c, "9c4d"_c, "9c4s"_c, "KhQs"_c,

          "Th6s"_c, "Th6c"_c, "Th6d"_c, "Td6c"_c, "Td6h"_c, "Td6s"_c, "Ts6h"_c,
          "Ts6d"_c, "Ts6c"_c, "Tc6h"_c, "Tc6d"_c, "Tc6s"_c,

          "Th5s"_c, "Th5c"_c, "Th5d"_c, "Td5c"_c, "Td5h"_c, "Td5s"_c, "Ts5h"_c,
          "Ts5d"_c, "Ts5c"_c, "Tc5h"_c, "Tc5d"_c, "Tc5s"_c,

          "Th4s"_c, "Th4c"_c, "Th4d"_c, "Td4c"_c, "Td4h"_c, "Td4s"_c, "Ts4h"_c,
          "Ts4d"_c, "Ts4c"_c, "Tc4h"_c, "Tc4d"_c, "Tc4s"_c,

          "AhKh"_c, "AsKs"_c, "AcKc"_c, "AdKd"_c, "AhQh"_c, "AdQd"_c, "AsQs"_c,
          "AcQc"_c, "AhJh"_c, "AdJd"_c, "AsJs"_c, "AcJc"_c,

          "AhKs"_c, "AhKc"_c, "AhKd"_c, "AdKc"_c, "AdKh"_c, "AdKs"_c, "AsKh"_c,
          "AsKd"_c, "AsKc"_c, "AcKh"_c, "AcKd"_c, "AcKs"_c, "AhQs"_c, "AhQc"_c,
          "AhQd"_c, "AdQc"_c, "AdQh"_c, "AdQs"_c, "AsQh"_c, "AsQd"_c, "AsQc"_c,
          "AcQh"_c, "AcQd"_c, "AcQs"_c, "AhJs"_c, "AhJc"_c, "AhJd"_c, "AdJc"_c,
          "AdJh"_c, "AdJs"_c, "AsJh"_c, "AsJd"_c, "AsJc"_c, "AcJh"_c, "AcJd"_c,
          "AcJs"_c,

          "AhAd"_c, "AhAs"_c, "AhAc"_c, "AdAs"_c, "AdAc"_c, "AcAs"_c, "KhKd"_c,
          "KhKs"_c, "KhKc"_c, "KdKs"_c, "KdKc"_c, "KcKs"_c, "QhQd"_c, "QhQs"_c,
          "QhQc"_c, "QdQs"_c, "QdQc"_c, "QcQs"_c, "7h7d"_c, "7h7s"_c, "7h7c"_c,
          "7d7s"_c, "7d7c"_c, "7c7s"_c, "6h6d"_c, "6h6s"_c, "6h6c"_c, "6d6s"_c,
          "6d6c"_c, "6c6s"_c, "5h5d"_c, "5h5s"_c, "5h5c"_c, "5d5s"_c, "5d5c"_c,
          "5c5s"_c, "4h4d"_c, "4h4s"_c, "4h4c"_c, "4d4s"_c, "4d4c"_c, "4c4s"_c,
          "2h2d"_c, "2h2s"_c, "2h2c"_c, "2d2s"_c, "2d2c"_c, "2c2s"_c});

      CHECK(reduced == sorted_vector({"AA-QQ"_re,
                                      "77-44"_re,
                                      "22"_re,
                                      "AKo-AJo"_re,
                                      "KQo-QJo"_re,
                                      "AKs-AJs"_re,
                                      "96s-94s"_re,
                                      "96o-94o"_re,
                                      "T6o-T4o"_re,
                                      "T6s-T4s"_re}));
    }
  }
}

TEST_CASE("range tests", "[range]")
{
  using namespace prc::literals;
  using namespace Catch::literals;

  SECTION("unassigned range")
  {
    SECTION("1 subrange, 1 elem, full weight")
    {
      prc::range all{"all", {{100.0, prc::any_two()}}};
      prc::range pairs{"pairs", {{100.0, {"22+"_re}}}};
      all.add_subrange(pairs);

      std::vector<prc::range::weighted_elems> expected{
          {100, remove_elems(prc::any_two(), {"22+"_re})}};
      auto const unassigned = unassigned_elems(all);
      CHECK(unassigned == expected);
    }

    SECTION("1 subrange, 1 elem, different weight")
    {
      prc::range all{"all", {{100.0, prc::any_two()}}};
      prc::range pairs{"pairs", {{40.5, {"22+"_re}}}};
      all.add_subrange(pairs);

      std::vector<prc::range::weighted_elems> expected{
          {59.5, {"22+"_re}}, {100, remove_elems(prc::any_two(), {"22+"_re})}};
      auto const unassigned = unassigned_elems(all);
      CHECK(unassigned == expected);
    }

    SECTION("1 subrange, 3 elem, different weight")
    {
      prc::range all{"all", {{100.0, prc::any_two()}}};
      prc::range pairs{
          "pairs",
          {{40.5, {"22-55"_re}}, {72.12, {"66"_re}}, {100.0, {"77+"_re}}}};
      all.add_subrange(pairs);

      std::vector<prc::range::weighted_elems> expected{
          {27.88, {"66"_re}},
          {59.5, {"22-55"_re}},
          {100, remove_elems(prc::any_two(), {"22+"_re})}};
      auto const unassigned = unassigned_elems(all);

      REQUIRE(unassigned.size() == expected.size());
      for (auto i = 0; i < unassigned.size(); ++i)
      {
        CHECK(unassigned[i].weight ==
              Catch::Detail::Approx(expected[i].weight));
        CHECK(unassigned[i].elems == expected[i].elems);
      }
    }

    SECTION("2 subrange, 1 elem, different weight")
    {
      prc::range all{"all", {{100.0, prc::any_two()}}};
      prc::range pairs{"pairs", {{40.5, {"22+"_re}}}};
      prc::range aces{"aces", {{20.5, {"AA"_re}}}};
      all.add_subrange(pairs);
      all.add_subrange(aces);

      std::vector<prc::range::weighted_elems> expected{
          {39.0, {"AA"_re}},
          {59.5, {"22-KK"_re}},
          {100, remove_elems(prc::any_two(), {"22+"_re})}};
      auto const unassigned = unassigned_elems(all);
      CHECK(unassigned == expected);
    }

    SECTION("everything assigned")
    {
      prc::range all{"all", {{100.0, prc::any_two()}}};
      prc::range aces{"aces", {{20.5, {"AA"_re}}}};
      all.add_subrange(all);

      auto const unassigned = unassigned_elems(all);
      CHECK(unassigned.empty());
    }
  }
}
