#include <prc/equilab/serialize.hpp>

#include <prc/detail/unicode.hpp>

#include <boost/algorithm/string/join.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

using namespace std::string_literals;

namespace prc::equilab
{
namespace
{
std::string rgb_to_string(int rgb)
{
  char buf[10];
  auto const r = (rgb >> 16) & 0xFF;
  auto const g = (rgb >> 8) & 0xFF;
  auto const b = rgb & 0xFF;
  std::snprintf(&buf[0], 10, "%03d%03d%03d", r, g, b);
  return buf;
}

inline auto const delim = "\xc2\xb0"s;

class serializer
{
public:
  std::string operator()(prc::folder const& f) const
  {
    std::string content(_depth, '.');
    content += f.name() + '\n';
    _depth++;
    for (auto const& entry : f.entries())
      content += boost::variant2::visit(*this, entry);
    _depth--;
    return content;
  }

  std::string operator()(std::vector<prc::range> const& subranges,
                         int& current_index,
                         int parent_index,
                         int nesting_index) const
  {
    std::string content;
    std::vector<int> subrange_indexes(subranges.size());
    std::iota(
        subrange_indexes.begin(), subrange_indexes.end(), current_index + 1);
    for (auto const& subrange : subranges)
    {
      current_index++;
      content += (*this)(subrange.elems());
      content += delim + std::to_string(current_index) + delim +
                 std::to_string(parent_index) + delim +
                 rgb_to_string(subrange.rgb()) + delim + subrange.name() +
                 delim + std::to_string(nesting_index) + delim;
    }
    for (auto i = 0; i < subranges.size(); ++i)
    {
      if (!subranges[i].subranges().empty())
      {
        content += (*this)(subranges[i].subranges(),
                           current_index,
                           subrange_indexes[i],
                           nesting_index + 1);
      }
    }
    return content;
  }

  std::string operator()(prc::range const& r) const
  {
    std::string content(_depth, '.');
    content += r.name() + " {";
    content += (*this)(r.elems());
    if (!r.subranges().empty())
    {
      content += delim + '0' + delim + '0' + delim + rgb_to_string(0xc0c0c0) +
                 delim + delim + '0' + delim;
      auto idx = 0;
      content += (*this)(r.subranges(), idx, 0, 0);
    }
    content += "}\n";
    return content;
  }

  std::string operator()(prc::range::weighted_elems const& we) const
  {
    std::stringstream wss;
    wss << std::fixed << std::setprecision(6) << we.weight;
    auto const weight_str = wss.str();
    if (we.elems == any_two())
      return weight_str + ":random";
    std::vector<std::string> strs;
    for (auto const& elem : we.elems)
    {
      std::stringstream ss;
      ss << elem;
      strs.push_back(ss.str());
    }
    return weight_str + ":" + boost::algorithm::join(strs, ",");
  }

  std::string operator()(
      std::vector<prc::range::weighted_elems> const& elems) const
  {
    std::vector<std::string> strs;
    for (auto const& elem : elems)
      strs.push_back((*this)(elem));
    auto ret = boost::algorithm::join(strs, ",");
    return ret;
  }

private:
  int mutable _depth{1};
};
}

std::u16string serialize(prc::folder const& f)
{
  std::string content = "[Userdefined]\n";
  for (auto const& entry : f.entries())
    content += boost::variant2::visit(serializer{}, entry);
  return detail::utf8_to_utf16le(content);
}
}
