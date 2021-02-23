#include <prc/combo.hpp>

#include <prc/detail/unicode.hpp>
#include <prc/hand.hpp>
#include <prc/parser/api.hpp>
#include <prc/range_elem.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <tuple>

namespace prc
{
namespace
{
class hand_range_expander
{
public:
  hand_range_expander(std::back_insert_iterator<std::vector<hand>> out)
    : _out(out)
  {
  }

  void operator()(paired_hand const& from, paired_hand const& to) const
  {
    auto from_int = static_cast<int>(from.rank());
    auto const to_int = static_cast<int>(to.rank());

    while (from_int != to_int)
      *_out = hand{paired_hand{static_cast<rank>(from_int++)}};
    *_out = hand{paired_hand{static_cast<rank>(to_int)}};
  }

  void operator()(unpaired_hand const& from, unpaired_hand const& to) const
  {
    auto from_high_int = static_cast<int>(from.high());
    auto from_low_int = static_cast<int>(from.low());
    auto to_high_int = static_cast<int>(to.high());
    auto to_low_int = static_cast<int>(to.low());

    if (from_high_int == to_high_int)
    {
      while (from_low_int != to_low_int)
      {
        *_out = hand{unpaired_hand{from.high(),
                                   static_cast<rank>(from_low_int++),
                                   static_cast<suitedness>(from.suited())}};
      }
      *_out = hand{unpaired_hand{from.high(),
                                 static_cast<rank>(from_low_int),
                                 static_cast<suitedness>(from.suited())}};
    }
    // 98s+ type of hands
    else
    {
      while (from_high_int != to_high_int)
      {
        *_out = hand{unpaired_hand{static_cast<rank>(from_high_int++),
                                   static_cast<rank>(from_low_int++),
                                   static_cast<suitedness>(from.suited())}};
      }
      *_out = hand{unpaired_hand{static_cast<rank>(from_high_int),
                                 static_cast<rank>(from_low_int),
                                 static_cast<suitedness>(from.suited())}};
    }
  }

  void operator()(hand const& from, hand const& to) const
  {
    if (auto f = from.get_if<paired_hand>())
      (*this)(*f, to.get<paired_hand>());
    else
      (*this)(from.get<unpaired_hand>(), to.get<unpaired_hand>());
  }

private:
  std::back_insert_iterator<std::vector<hand>> mutable _out;
};

class hand_expander
{
public:
  hand_expander(std::back_insert_iterator<std::vector<hand>> out) : _out{out}
  {
  }

  void operator()(combo const& c) const
  {
    throw std::runtime_error("combos are not supported here");
  }

  void operator()(hand const& h) const
  {
    *_out = h;
  }

  void operator()(hand_range const& hr) const
  {
    hand_range_expander{_out}(hr.from(), hr.to());
  }

private:
  std::back_insert_iterator<std::vector<hand>> mutable _out;
};

class combo_expander
{
public:
  combo_expander(std::back_insert_iterator<std::vector<combo>> out) : _out{out}
  {
  }

  void operator()(combo const& c) const
  {
    *_out = c;
  }

  void operator()(paired_hand const& h) const
  {
    for (auto i = 0; i < 4; ++i)
    {
      for (auto j = i + 1; j < 4; ++j)
      {
        *_out = {card{h.rank(), static_cast<suit>(i)},
                 card{h.rank(), static_cast<suit>(j)}};
      }
    }
  }

  void operator()(unpaired_hand const& uh) const
  {
    if (uh.suited())
    {
      for (auto i = 0; i < 4; ++i)
      {
        *_out = {card{uh.high(), static_cast<suit>(i)},
                 card{uh.low(), static_cast<suit>(i)}};
      }
    }
    else
    {
      for (auto i = 0; i < 4; ++i)
      {
        for (auto j = 0; j < 4; ++j)
        {
          if (i == j)
            continue;
          *_out = {card{uh.high(), static_cast<suit>(i)},
                   card{uh.low(), static_cast<suit>(j)}};
        }
      }
    }
  }

  void operator()(hand const& h) const
  {
    h.visit(*this);
  }

  void operator()(hand_range const& hr) const
  {
    std::vector<hand> hands;
    hand_range_expander{std::back_inserter(hands)}(hr.from(), hr.to());
    for (auto const& h : hands)
      ((*this)(h));
  }

private:
  std::back_insert_iterator<std::vector<combo>> mutable _out;
};

struct paired_hands_pred
{
  bool operator()(paired_hand lhs, paired_hand rhs) const
  {
    return (static_cast<int>(rhs.rank()) - static_cast<int>(lhs.rank())) > 1;
  };
};

class unpaired_hands_pred
{
public:
  bool operator()(unpaired_hand const& lhs, unpaired_hand const& rhs) const
  {
    auto const lhs_high = static_cast<int>(lhs.high());
    auto const lhs_low = static_cast<int>(lhs.low());
    auto const rhs_high = static_cast<int>(rhs.high());
    auto const rhs_low = static_cast<int>(rhs.low());

    if (_first_call)
    {
      _first_call = false;
      if (lhs_high - lhs_low == 1 && rhs_high - rhs_low == 1)
        _diagonal = true;
    }
    if (_diagonal)
      return !(rhs_high - lhs_high == 1 && rhs_low - lhs_low == 1);
    if (rhs_high == lhs_high)
      return rhs_low - lhs_low > 1;
    return true;
  }

private:
  bool mutable _first_call{true};
  bool mutable _diagonal{false};
  unpaired_hand _start_hand;
};

// only called when adjacent_find failed
struct hand_range_validator
{
  bool operator()(paired_hand const& lhs, paired_hand const& rhs) const
  {
    return (static_cast<int>(rhs.rank()) - static_cast<int>(lhs.rank())) >= 1;
  }

  bool operator()(unpaired_hand const& lhs, unpaired_hand const& rhs) const
  {
    auto const lhs_high = static_cast<int>(lhs.high());
    auto const lhs_low = static_cast<int>(lhs.low());
    auto const rhs_high = static_cast<int>(rhs.high());
    auto const rhs_low = static_cast<int>(rhs.low());

    if (rhs_high == lhs_high)
      return rhs_low - lhs_low >= 1;
    return rhs_high - lhs_high == rhs_low - lhs_low;
  }
};

template <typename BinaryPredicate, typename Iterator, typename Sentinel>
void reduce_hand_ranges(Iterator it,
                        Sentinel const s,
                        std::vector<range_elem>& out)
{
  while (it != s)
  {
    auto from_it = it;
    it = std::adjacent_find(it, s, BinaryPredicate{});
    if (it == s)
    {
      // happens with only two adjacent pairs (e.g. AA-KK)
      if (from_it == it - 1)
        out.emplace_back(hand{*from_it});
      else
        out.emplace_back(hand_range{hand{*from_it}, hand{*(it - 1)}});
      return;
    }
    if (it == from_it)
      out.emplace_back(hand{*from_it});
    else if (hand_range_validator{}(*from_it, *it))
      out.emplace_back(hand_range{hand{*from_it}, hand{*it}});
    else
    {
      out.emplace_back(hand{*from_it});
      out.emplace_back(hand{*it});
    }
    it++;
  }
}

template <typename Iterator, typename Sentinel>
std::vector<range_elem> reduce_pairs(Iterator it, Sentinel s)
{
  std::vector<range_elem> ret;
  std::map<rank, std::vector<combo>> combos_by_rank;

  std::for_each(
      it, s, [&](auto& e) { combos_by_rank[e.high().rank()].push_back(e); });

  std::vector<paired_hand> pairs;
  for (auto const& [r, combos] : combos_by_rank)
  {
    if (combos.size() == 6)
      pairs.emplace_back(r);
    else
    {
      std::transform(
          combos.begin(), combos.end(), std::back_inserter(ret), [](auto& c) {
            return range_elem{c};
          });
    }
  }

  reduce_hand_ranges<paired_hands_pred>(pairs.begin(), pairs.end(), ret);
  return ret;
}

template <typename Iterator, typename Sentinel>
std::vector<range_elem> reduce_suited(Iterator it, Sentinel s)
{
  std::vector<range_elem> ret;
  std::map<std::pair<rank, rank>, std::vector<combo>> combos_by_ranks;

  std::for_each(it, s, [&](auto& e) {
    combos_by_ranks[std::make_pair(e.high().rank(), e.low().rank())].push_back(
        e);
  });

  std::vector<unpaired_hand> unpaired_hands;
  for (auto const& [ranks, combos] : combos_by_ranks)
  {
    auto const [high, low] = ranks;
    if (combos.size() == 4)
      unpaired_hands.emplace_back(high, low, suitedness::suited);
    else
    {
      std::transform(
          combos.begin(), combos.end(), std::back_inserter(ret), [](auto& c) {
            return range_elem{c};
          });
    }
  }

  reduce_hand_ranges<unpaired_hands_pred>(
      unpaired_hands.begin(), unpaired_hands.end(), ret);
  return ret;
}

template <typename Iterator, typename Sentinel>
std::vector<range_elem> reduce_offsuit(Iterator it, Sentinel s)
{
  std::vector<range_elem> ret;
  std::map<std::pair<rank, rank>, std::vector<combo>> combos_by_ranks;

  std::for_each(it, s, [&](auto& e) {
    combos_by_ranks[std::make_pair(e.high().rank(), e.low().rank())].push_back(
        e);
  });

  std::vector<unpaired_hand> unpaired_hands;
  for (auto const& [ranks, combos] : combos_by_ranks)
  {
    auto const [high, low] = ranks;
    if (combos.size() == 12)
      unpaired_hands.emplace_back(high, low, suitedness::offsuit);
    else
    {
      std::transform(
          combos.begin(), combos.end(), std::back_inserter(ret), [](auto& c) {
            return range_elem{c};
          });
    }
  }

  reduce_hand_ranges<unpaired_hands_pred>(
      unpaired_hands.begin(), unpaired_hands.end(), ret);
  return ret;
}

auto const comp = [](auto& lhs, auto& rhs) {
  if (lhs.high().rank() < rhs.high().rank())
    return true;
  if (lhs.high().rank() > rhs.high().rank())
    return false;
  if (lhs.low().rank() < rhs.low().rank())
    return true;
  if (lhs.low().rank() > rhs.low().rank())
    return false;
  if (lhs.high().suit() < rhs.high().suit())
    return true;
  if (lhs.high().suit() > rhs.high().suit())
    return false;
  return lhs.low().suit() < rhs.low().suit();
};

template <typename T>
std::vector<T> sort_unique(std::vector<T> const& v)
{
  auto ret = v;
  std::sort(ret.begin(), ret.end(), comp);
  auto const it = std::unique(ret.begin(), ret.end());
  ret.erase(it, ret.end());
  return ret;
}
}

combo::combo(card lhs, card rhs) : _high(lhs), _low(rhs)
{
  if (lhs == rhs)
  {
    throw std::runtime_error{
        "combo cannot be composed of two exact same cards"};
  }
  if (lhs < rhs)
    std::swap(_high, _low);
}

combo::combo(parser::ast::combo const& c)
  : combo(card{c.first_card}, card{c.second_card})
{
}

card const& combo::high() const
{
  return _high;
}

card const& combo::low() const
{
  return _low;
}

std::string combo::string() const
{
  return _high.string() + _low.string();
}

bool combo::suited() const
{
  return _high.suit() == _low.suit();
}

bool combo::offsuit() const
{
  return !suited();
}

bool combo::paired() const
{
  return _high.rank() == _low.rank();
}

bool operator==(combo const& lhs, combo const& rhs) noexcept
{
  return std::tie(lhs.high(), lhs.low()) == std::tie(rhs.high(), rhs.low());
}

bool operator!=(combo const& lhs, combo const& rhs) noexcept
{
  return !(lhs == rhs);
}

bool operator<(combo const& lhs, combo const& rhs) noexcept
{
  return std::tie(lhs.high(), lhs.low()) < std::tie(rhs.high(), rhs.low());
}

std::ostream& operator<<(std::ostream& os, combo const& c)
{
  return os << c.high() << c.low();
}

std::vector<prc::combo> expand_combos(range_elem const& elem)
{
  std::vector<prc::combo> ret;
  elem.visit(combo_expander{std::back_inserter(ret)});
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::vector<prc::combo> expand_combos(std::vector<range_elem> const& elems)
{
  std::vector<prc::combo> ret;
  combo_expander exp{std::back_inserter(ret)};
  for (auto const& elem : elems)
    elem.visit(exp);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

std::vector<prc::hand> expand_hands(std::vector<range_elem> const& elems)
{
  std::vector<prc::hand> ret;
  hand_expander exp{std::back_inserter(ret)};
  for (auto const& elem : elems)
    elem.visit(exp);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

std::vector<range_elem> reduce_combos(std::vector<combo> const& combos)
{
  auto vec = sort_unique(combos);

  auto const pairs_it = std::stable_partition(
      vec.begin(), vec.end(), [](auto& e) { return e.paired(); });
  auto const suited_it = std::stable_partition(
      pairs_it, vec.end(), [](auto& e) { return e.suited(); });

  auto ret = reduce_pairs(vec.begin(), pairs_it);
  auto suited = reduce_suited(pairs_it, suited_it);
  auto offsuit = reduce_offsuit(suited_it, vec.end());

  ret.insert(ret.end(), suited.begin(), suited.end());
  ret.insert(ret.end(), offsuit.begin(), offsuit.end());
  std::sort(ret.begin(), ret.end());
  return ret;
}

std::vector<range_elem> const& any_two()
{
  static std::vector const vec = [] {
    std::vector v{"22+"_re,  "A2o+"_re, "K2o+"_re, "Q2o+"_re, "J2o+"_re,
                  "T2o+"_re, "92o+"_re, "82o+"_re, "72o+"_re, "62o+"_re,
                  "52o+"_re, "42o+"_re, "32o"_re,  "A2s+"_re, "K2s+"_re,
                  "Q2s+"_re, "J2s+"_re, "T2s+"_re, "92s+"_re, "82s+"_re,
                  "72s+"_re, "62s+"_re, "52s+"_re, "42s+"_re, "32s"_re};
    std::sort(v.begin(), v.end());
    return v;
  }();
  return vec;
};

std::vector<prc::combo> const& any_two_combos()
{
  static auto const vec = expand_combos(any_two());
  return vec;
}

inline namespace literals
{
combo operator"" _c(char const* str, std::size_t n)
{
  namespace x3 = boost::spirit::x3;

  auto const s = detail::utf8_to_utf32(std::string_view{str, n});

  x3::error_handler<std::u32string::const_iterator> error_handler(
      s.begin(), s.end(), std::cerr);
  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[x3::expect[parser::combo()] > x3::eoi];
  parser::ast::combo c;
  x3::phrase_parse(s.begin(), s.end(), ctx, x3::unicode::space, c);
  return combo{c};
}
}
}
