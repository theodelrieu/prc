#include <prc/combo.hpp>
#include <prc/pio/serialize.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace std::string_literals;

namespace prc::pio
{
namespace
{
std::size_t index_of(combo const& c)
{
  auto const& combos = any_two_combos();
  auto it = std::lower_bound(combos.begin(), combos.end(), c);
  return std::distance(combos.begin(), it);
}

void write_combo_weights(std::string& content,
                         std::vector<range::weighted_elems> const& elems)
{
  auto const& combos = any_two_combos();
  std::vector<double> weights(1326);
  for (auto const& [w, e] : elems)
  {
    for (auto const& c : expand_combos(e))
      weights[index_of(c)] = w / 100.0;
  }

  std::stringstream ss;
  ss << std::fixed << std::setprecision(3);
  for (auto const w : weights)
  {
    // avoid trailing zeros
    if (w == 1.0 || w == 0.0)
      ss << int(w) << ' ';
    else
      ss << w << ' ';
  }
  content += ss.str();
  content.pop_back();
}
}

std::string serialize(prc::range const& r)
{
  auto content = "PreflopCharts\n"s;
  write_combo_weights(content, r.elems());
  content += '\n';
  for (auto const& sub : r.subranges())
  {
    content += "True\n" + std::to_string(sub.rgb()) + "\t" + sub.name() + "\t";
    write_combo_weights(content, sub.elems());
    content += '\n';
  }
  content.pop_back();
  return content;
}
}
