#include <prc/range.hpp>

#include <chrono>

#include <algorithm>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <utility>

#include <boost/algorithm/string/join.hpp>

namespace prc
{
namespace
{
constexpr auto nb_total_combos = 1326;
constexpr auto minimum_weight = 0.001;

std::vector<range::weighted_elems> weights_to_weighted_elems(
    std::vector<double> const& weights)
{
  std::map<double, std::vector<combo>> m;
  auto const& any_two = any_two_combos();

  for (auto i = 0; i < nb_total_combos; ++i)
  {
    // do not include folded combos ever
    // it can be reconstituted by expanding combos of all subranges, then
    // set_difference between that and the parent combos
    if (weights[i] > minimum_weight)
      m[weights[i] * 100.0].push_back(any_two[i]);
  }

  std::vector<range::weighted_elems> ret;
  for (auto const& [weight, combos] : m)
    ret.push_back({weight, reduce_combos(combos)});
  return ret;
}

std::vector<range::weighted_elems> weights_to_weighted_elems(
    std::vector<double> const& weights,
    std::vector<double> const& parent_weights)
{
  std::vector<double> adjusted_weights;

  // pio weights are absolute, e.g. parent has 0.8, child has 0.7, it means 70%
  // of 100%, not 70% of 80%
  // equilab stores relative values and transform them in absolute ones when the
  // user selects a range to bring to the main window
  //
  // I do like equilab to allow easy replacing of parent ranges later on
  std::transform(weights.begin(),
                 weights.end(),
                 parent_weights.begin(),
                 std::back_inserter(adjusted_weights),
                 [](auto a, auto b) {
                   auto const margin = 1.0 - b;
                   return a + margin;
                 });
  return weights_to_weighted_elems(adjusted_weights);
}

using weight_by_combo_t = std::map<prc::combo, double>;

weight_by_combo_t weight_by_combos(
    std::vector<range::weighted_elems> const& elems)
{
  weight_by_combo_t ret;
  for (auto const& [weight, e] : elems)
  {
    for (auto const& combo : expand_combos(e))
    {
      auto& w = ret[combo];
      w += weight;
      if (w + minimum_weight >= 100.0)
        w = 100.0;
    }
  }
  return ret;
}

std::vector<range::weighted_elems> unassigned_combos_to_weighted_elems(
    weight_by_combo_t const& combos)
{
  std::map<double, std::vector<prc::combo>> m;
  std::vector<range::weighted_elems> ret;

  for (auto const& [combo, weight] : combos)
  {
    if (weight > minimum_weight)
      m[weight].push_back(combo);
  }
  for (auto const& [weight, combos] : m)
    ret.push_back(range::weighted_elems{weight, reduce_combos(combos)});
  return ret;
}

std::vector<range::weighted_elems> equilab_weighted_hands_to_weighted_elems(
    std::vector<equilab::parser::ast::weighted_hands> const& weighted_hands)
{
  std::vector<range::weighted_elems> ret;
  for (auto const& [weight, hands] : weighted_hands)
  {
    if (weight <= minimum_weight)
      continue;
    range::weighted_elems e{weight, {}};
    std::transform(
        hands.begin(), hands.end(), std::back_inserter(e.elems), [](auto& e) {
          return prc::range_elem{e};
        });
    std::sort(e.elems.begin(), e.elems.end());
    ret.push_back(std::move(e));
  }
  return ret;
}
template <typename Iterator>
Iterator order_equilab_groups(Iterator begin, Iterator end)
{
  std::sort(begin, end, [](auto& lhs, auto& rhs) {
    if (lhs.info.value().nesting_index != rhs.info.value().nesting_index)
      return lhs.info.value().nesting_index < rhs.info.value().nesting_index;
    return lhs.info.value().index < rhs.info.value().index;
  });
  std::stable_sort(begin, end, [](auto& lhs, auto& rhs) {
    return lhs.info.value().nesting_index > rhs.info.value().nesting_index;
  });
  return std::find_if(
      begin, end, [](auto& g) { return g.info.value().nesting_index == 0; });
}

template <typename Iterator, typename Sentinel>
std::vector<range> nest_equilab_ranges(Iterator begin, Sentinel end)
{
  // far from being optimized nor pretty, but heh
  auto const first_non_nested = order_equilab_groups(begin, end);

  std::map<int, int> index_to_pos;
  for (auto it = begin; it != end; ++it)
    index_to_pos[it->info->index] = std::distance(begin, it);

  std::vector<range> subranges;
  std::transform(begin, end, std::back_inserter(subranges), [](auto& g) {
    auto elems = equilab_weighted_hands_to_weighted_elems(g.weighted_hands);
    return range{g.info->name, std::move(elems), g.info->rgb};
  });
  for (auto it = begin; it != end; ++it)
  {
    if (it->info->nesting_index > 0)
    {
      auto const parent_pos = index_to_pos[it->info->parent_index];
      // copy, to be able to erase remove after
      subranges[parent_pos].add_subrange(
          subranges[index_to_pos[it->info->index]]);
    }
  }
  auto const first_non_nested_pos = std::distance(begin, first_non_nested);
  return std::vector<range>(
      std::make_move_iterator(subranges.begin() + first_non_nested_pos),
      std::make_move_iterator(subranges.end()));
}
}

range::range(std::string name,
             std::vector<weighted_elems> elems,
             int rgb,
             std::vector<range> subranges)
  : _name(std::move(name)),
    _elems(std::move(elems)),
    _rgb(rgb),
    _subranges(std::move(subranges))
{
}

range::range(equilab::parser::ast::range const& r) : _name(r.name), _rgb(0)
{
  if (r.groups.empty())
    throw std::runtime_error{"there must be at least one group in range"};
  auto& base_range = r.groups.front();
  _elems = equilab_weighted_hands_to_weighted_elems(base_range.weighted_hands);
  if (r.groups.size() > 1)
  {
    std::vector groups(r.groups.begin() + 1, r.groups.end());
    _subranges = nest_equilab_ranges(groups.begin(), groups.end());
  }
}

range::range(pio::parser::ast::range const& r)
{
  if (r.base_range.weights.size() != nb_total_combos)
    throw std::runtime_error{"base_range does not have 1326 weights"};
  _elems = weights_to_weighted_elems(r.base_range.weights);
  for (auto const& s : r.subranges)
  {
    if (s.weights.size() != nb_total_combos)
      throw std::runtime_error{"subrange does not have 1326 weights"};
    auto sub_elems = weights_to_weighted_elems(s.weights, r.base_range.weights);
    if (!sub_elems.empty())
      add_subrange({s.name, std::move(sub_elems), s.rgb});
  }
}

void range::add_subrange(range const& r)
{
  _subranges.push_back(r);
}

void range::add_subrange(range&& r)
{
  _subranges.push_back(std::move(r));
}

range* range::find_subrange(std::string const& name)
{
  auto it = std::find_if(_subranges.begin(), _subranges.end(), [&](auto& s) {
    return s.name() == name;
  });
  if (it == _subranges.end())
    return nullptr;
  return std::addressof(*it);
}

range const* range::find_subrange(std::string const& name) const
{
  return const_cast<range const*>(
      (*const_cast<range const*>(this)).find_subrange(name));
}

void range::set_rgb(int rgb)
{
  _rgb = rgb;
}

void range::set_name(std::string name)
{
  _name = std::move(name);
}

void range::set_elems(std::vector<weighted_elems> elems)
{
  _elems = std::move(elems);
}

std::string const& range::name() const
{
  return _name;
}

auto range::elems() const -> std::vector<weighted_elems> const&
{
  return _elems;
}

int range::rgb() const
{
  return _rgb;
}

std::vector<range> const& range::subranges() const
{
  return _subranges;
}

std::vector<range>& range::subranges()
{
  return _subranges;
}

bool operator==(range const& lhs, range const& rhs)
{
  return lhs.rgb() == rhs.rgb() &&
         std::tie(lhs.name(), lhs.elems(), lhs.subranges()) ==
             std::tie(rhs.name(), rhs.elems(), rhs.subranges());
}

bool operator!=(range const& lhs, range const& rhs)
{
  return !(lhs == rhs);
}

bool operator==(range::weighted_elems const& lhs,
                range::weighted_elems const& rhs)
{
  return std::tie(lhs.weight, lhs.elems) == std::tie(rhs.weight, rhs.elems);
}

bool operator!=(range::weighted_elems const& lhs,
                range::weighted_elems const& rhs)
{
  return !(lhs == rhs);
}

std::vector<range::weighted_elems> unassigned_elems(range const& r)
{
  if (r.subranges().empty())
    return {};
  auto current = weight_by_combos(r.elems());
  for (auto const& sub : r.subranges())
  {
    for (auto const& [combo, weight] : weight_by_combos(sub.elems()))
    {
      auto& w = current[combo];
      w -= weight;
      if (w <= minimum_weight)
        w = 0;
    }
  }

  return unassigned_combos_to_weighted_elems(current);
}

std::vector<range::weighted_elems> adjust_weights(
    std::vector<range::weighted_elems> const& base_range_elems, range const& r)
{
  auto const base_range_combos = weight_by_combos(base_range_elems);
  auto const range_combos = weight_by_combos(r.elems());

  std::map<double, std::vector<prc::combo>> tmp;
  for (auto const& [combo, weight] : range_combos)
  {
    if (auto it = base_range_combos.find(combo); it != base_range_combos.end())
      tmp[(it->second * weight) / 100.0].push_back(combo);
  }
  std::vector<range::weighted_elems> ret;
  for (auto const& [weight, combos] : tmp)
    ret.push_back(range::weighted_elems{weight, reduce_combos(combos)});
  return ret;
}

std::ostream& operator<<(std::ostream& os, range::weighted_elems const& elems)
{
  std::vector<std::string> strs;
  for (auto const& elem : elems.elems)
  {
    std::stringstream ss;
    ss << elem;
    strs.push_back(ss.str());
  }
  os << elems.weight << ":";
  os << boost::algorithm::join(strs, ",");
  return os;
}
}
