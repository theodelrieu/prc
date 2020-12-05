#include "actions.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <prc/parser/as_type.hpp>
#include <prc/range.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/fusion/adapted/std_pair.hpp>

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;

namespace prc::actions
{
namespace
{
x3::rule<struct range_name_class,
         std::vector<std::pair<std::string, std::string>>> const _range_name =
    "range name";

auto const _position = parser::as<
    std::string>[x3::lexeme[+(x3::upper | x3::digit | x3::char_('+'))]];

auto const _action = x3::lexeme[+(x3::char_ - '_')];
auto const _range_name_def =
    parser::as<std::pair<std::string, std::string>>[_position >> '_' >>
                                                    _action] %
    '_';

BOOST_SPIRIT_DEFINE(_range_name);

struct flattened_range
{
  fs::path parent_path;
  prc::folder* parent_folder;
  prc::range* range;
};

struct parent_child_range
{
  range* parent;
  range* child;
  fs::path parent_path;
  fs::path child_path;
  prc::folder* child_parent_folder;
  std::string subrange_name;
};

struct parent_range_info
{
  std::string name;
  std::string subrange_name;
};

bool has_vsxbet_folders(folder& root)
{
  auto it =
      std::find_if(root.entries().begin(), root.entries().end(), [](auto& f) {
        if (auto p = boost::variant2::get_if<folder>(&f))
          return boost::algorithm::contains(p->name(), "vs_");
        return false;
      });
  return it != root.entries().end();
}

void recurse_fill_ranges(folder& root,
                         fs::path const& current_path,
                         std::vector<flattened_range>& out)
{
  for (auto& entry : root.entries())
  {
    if (auto p = boost::variant2::get_if<range>(&entry))
      out.push_back({current_path / p->name(), &root, p});
    else
    {
      auto& subfolder = boost::variant2::get<folder>(entry);
      recurse_fill_ranges(subfolder, current_path / subfolder.name(), out);
    }
  }
}

std::vector<std::pair<std::string, std::string>> parse_range_name(
    std::string const& input)
{
  std::vector<std::pair<std::string, std::string>> ret;
  auto b = input.begin();
  auto r =
      x3::phrase_parse(b, input.end(), _range_name >> x3::eoi, x3::space, ret);
  if (!r || b != input.end())
  {
    std::cout << "Could not parse range name: " << input << ", skipping"
              << std::endl;
    return {};
  }
  return ret;
}

void replace_parent_range(parent_child_range& p)
{
  auto const& subranges = p.parent->subranges();
  auto const subrange_it =
      std::find_if(subranges.begin(), subranges.end(), [&](auto& s) {
        return s.name() == p.subrange_name;
      });
  if (subrange_it == subranges.end())
  {
    // TODO add fmt once conan-center-index boost 1.75.0 is fixed
    std::cout << p.parent_path << ": no subrange " << p.subrange_name
              << std::endl;

    std::cout << p.child_path << ": could not replace parent range"
              << std::endl;
    return;
  }
  auto const& parent_range_elems = subrange_it->elems();
  auto const adjusted_parent_elems =
      adjust_weights(parent_range_elems, *p.parent);
  auto adjusted_elems = adjust_weights(adjusted_parent_elems, *p.child);
  p.child->set_elems(std::move(adjusted_elems));
  std::cout << p.child_path << ": replaced parent range by subrange "
            << p.subrange_name << " of " << p.parent_path << std::endl;
}

void nest_range(parent_child_range& p)
{
  auto subrange = p.parent->find_subrange(p.subrange_name);
  if (!subrange)
  {
    // TODO add fmt once conan-center-index boost 1.75.0 is fixed
    std::cout << p.parent_path << ": no subrange " << p.subrange_name
              << std::endl;

    std::cout << p.child_path << ": could not nest" << std::endl;
    return;
  }

  auto const prefix = boost::algorithm::erase_all_copy(
      p.parent_path.filename().string(), "_strategy");
  auto const new_name = boost::algorithm::erase_all_copy(
      p.child_path.filename().string(), prefix + '_' + p.subrange_name + '_');
  // nesting is a bit weird in equilab, ranges have 100% of their parent
  std::vector<prc::combo> combos;
  for (auto const& [w, e] : p.child->elems())
  {
    auto c = prc::expand_combos(e);
    combos.insert(combos.end(), c.begin(), c.end());
  }
  auto new_range = *p.child;
  new_range.set_name(new_name);
  new_range.set_elems({{100.0, prc::reduce_combos(combos)}});
  // mimick equilab selection color
  new_range.set_rgb(0xff6ebaff);
  subrange->add_subrange(std::move(new_range));
  std::cout << p.child_path << ": nested into subrange " << p.subrange_name
            << " of " << p.parent_path << " as " << new_name << std::endl;
}

std::optional<parent_range_info> get_parent_range_info(std::string const& name)
{
  auto const parts = parse_range_name(name);
  if (parts.empty())
    return std::nullopt;
  auto const& position = parts.back().first;
  auto const pos_it =
      std::find_if(parts.rbegin() + 1, parts.rend(), [&position](auto& p) {
        return p.first == position;
      });
  if (pos_it == parts.rend())
    return std::nullopt;
  std::string parent_name;
  for (auto it = parts.begin(); it != pos_it.base() - 1; ++it)
  {
    auto const& [pos, action] = *it;
    parent_name += pos + '_' + action + '_';
  }
  parent_name += position + "_strategy";
  return {{std::move(parent_name), pos_it->second}};
}

std::vector<flattened_range> flatten_ranges(folder& f,
                                            fs::path const& current_path)
{
  std::vector<flattened_range> flattened_ranges;
  recurse_fill_ranges(f, current_path, flattened_ranges);

  std::sort(flattened_ranges.begin(),
            flattened_ranges.end(),
            [](auto& lhs, auto& rhs) {
              if (lhs.range->name().size() == rhs.range->name().size())
                return lhs.range->name() < rhs.range->name();
              return lhs.range->name().size() < rhs.range->name().size();
            });
  return flattened_ranges;
}

std::vector<parent_child_range> find_parent_ranges(
    std::vector<flattened_range> const& flattened_ranges)
{
  std::vector<parent_child_range> parent_ranges;

  for (auto it = flattened_ranges.rbegin(); it != flattened_ranges.rend(); ++it)
  {
    auto info = get_parent_range_info(it->range->name());
    if (info)
    {
      auto const parent_it =
          std::find_if(it + 1, flattened_ranges.rend(), [&info](auto& r) {
            return r.range->name() == info->name;
          });
      if (parent_it != flattened_ranges.rend())
      {
        parent_ranges.push_back({parent_it->range,
                                 it->range,
                                 parent_it->parent_path,
                                 it->parent_path,
                                 it->parent_folder,
                                 std::move(info->subrange_name)});
      }
    }
  }
  std::reverse(parent_ranges.begin(), parent_ranges.end());
  return parent_ranges;
}
}

inline namespace folder_actions
{
folder_action remove_empty_ranges()
{
  return [](folder& f, fs::path const& current_path) {
    auto& entries = f.entries();
    entries.erase(
        std::remove_if(entries.begin(),
                       entries.end(),
                       [&](auto& e) {
                         if (auto p = boost::variant2::get_if<range>(&e))
                         {
                           auto b = p->subranges().empty();
                           if (b)
                           {
                             std::cout << "Removing "
                                       << current_path / p->name() << std::endl;
                           }
                           return b;
                         }
                         return false;
                       }),
        entries.end());
    return true;
  };
}

folder_action fix_parent_ranges()
{
  return [](auto& folder, fs::path const& current_path) {
    if (!has_vsxbet_folders(folder))
      return true;
    auto const flattened_ranges = flatten_ranges(folder, current_path);
    auto parent_ranges = find_parent_ranges(flattened_ranges);

    for (auto& elem : parent_ranges)
      replace_parent_range(elem);
    return false;
  };
}

folder_action nest_parent_ranges()
{
  return [](auto& folder, fs::path const& current_path) {
    if (!has_vsxbet_folders(folder))
      return true;
    auto const flattened_ranges = flatten_ranges(folder, current_path);
    auto parent_ranges = find_parent_ranges(flattened_ranges);

    for (auto it = parent_ranges.rbegin(); it != parent_ranges.rend(); ++it)
      nest_range(*it);
    // we can remove now, iterators can be invalidated
    for (auto it = parent_ranges.rbegin(); it != parent_ranges.rend(); ++it)
      it->child_parent_folder->remove_entry(it->child_path.filename().string());
    return true;
  };
}
}

inline namespace range_actions
{
range_action replace_in_name(std::string const& old_str,
                             std::string const& new_str)
{
  return [old_str, new_str](range& r, fs::path const& abs_parent_path) {
    if (boost::algorithm::contains(r.name(), old_str))
    {
      std::cout << abs_parent_path / r.name() << ": rename to ";
      r.set_name(
          boost::algorithm::replace_all_copy(r.name(), old_str, new_str));
      std::cout << r.name() << std::endl;
    }
  };
}

range_action change_color(std::string const& range_name, int rgb)
{
  return [range_name, rgb](range& r, fs::path const& abs_parent_path) {
    if (r.name() == range_name && r.rgb() != rgb)
    {
      std::cout << abs_parent_path / r.name() << ": changing color from "
                << std::hex << r.rgb() << " to " << rgb << std::dec
                << std::endl;
      r.set_rgb(rgb);
    }
  };
}

range_action set_unassigned_to_subrange(std::string const& range_name, int rgb)
{
  return [range_name, rgb](range& r, fs::path const& abs_parent_path) {
    if (r.subranges().empty())
      return;
    for (auto& sub : r.subranges())
    {
      if (sub.name() == range_name)
        return;
    }

    auto unassigned = prc::unassigned_elems(r);
    if (!unassigned.empty())
    {
      r.add_subrange({range_name, std::move(unassigned), rgb});
      std::cout << abs_parent_path / r.name() << ": set unassigned range to "
                << range_name << std::endl;
    }
  };
}

range_action move_subrange_at_end(std::string const& range_name)
{
  return [range_name](range& r, fs::path const& abs_parent_path) {
    auto const end = r.subranges().end();
    auto it = std::find_if(r.subranges().begin(), end, [&](auto& s) {
      return s.name() == range_name;
    });
    if (it != end)
      std::rotate(it, it + 1, end);
  };
}
}
}