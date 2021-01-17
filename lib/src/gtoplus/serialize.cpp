#include <prc/gtoplus/serialize.hpp>

#include <prc/range_elem.hpp>

#include <boost/locale.hpp>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>

namespace prc::gtoplus
{
namespace
{
struct group_name_rgb
{
  std::string name;
  int rgb;
};

struct hand_info
{
  std::vector<std::pair<int, double>> index_to_ratio;
};
// TODO avoid copy pasting :D

class hand_range_expander
{
public:
  hand_range_expander(std::back_insert_iterator<std::vector<prc::hand>> out)
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
  std::back_insert_iterator<std::vector<prc::hand>> mutable _out;
};

class range_elem_expander
{
public:
  range_elem_expander(std::back_insert_iterator<std::vector<range_elem>> out)
    : _out{out}
  {
  }

  void operator()(combo const& c) const
  {
    throw std::runtime_error("combos are not supported here");
  }

  void operator()(hand const& h) const
  {
    *_out = prc::range_elem{h};
  }

  void operator()(hand_range const& hr) const
  {
    std::vector<hand> hands;
    hand_range_expander{std::back_inserter(hands)}(hr.from(), hr.to());
    for (auto const& h : hands)
      ((*this)(h));
  }

private:
  std::back_insert_iterator<std::vector<range_elem>> mutable _out;
};

std::string rgb_to_string(int rgb)
{
  char buf[12];
  auto const r = (rgb >> 16) & 0xFF;
  auto const g = (rgb >> 8) & 0xFF;
  auto const b = rgb & 0xFF;
  std::snprintf(&buf[0], 12, "%03d %03d %03d", r, g, b);
  return buf;
}

std::vector<prc::range_elem> expand_hands(
    std::vector<prc::range_elem> const& elems)
{
  std::vector<prc::range_elem> ret;
  range_elem_expander exp{std::back_inserter(ret)};
  for (auto const& elem : elems)
    elem.visit(exp);
  return ret;
}

void percents_to_ratios(hand_info& info)
{
  auto it = std::min_element(
      info.index_to_ratio.begin(),
      info.index_to_ratio.end(),
      [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });
  assert(it != info.index_to_ratio.end());
  hand_info tmp;
  for (auto const& [idx, weight] : info.index_to_ratio)
    tmp.index_to_ratio.emplace_back(idx, weight / it->second);
  info = std::move(tmp);
}

std::optional<hand_info> get_hand_info(
    prc::range_elem const& re,
    std::vector<prc::range> const& subranges,
    std::vector<group_name_rgb> const& group_names_rgbs)
{
  hand_info ret;

  for (auto const& sub : subranges)
  {
    for (auto const& [w, e] : sub.elems())
    {
      auto const hands = expand_hands(e);
      auto const it = std::find(hands.begin(), hands.end(), re);
      if (it != hands.end())
      {
        auto const group_name_it = std::find_if(
            group_names_rgbs.begin(), group_names_rgbs.end(), [&](auto& g) {
              return g.rgb == sub.rgb();
            });
        if (group_name_it == group_names_rgbs.end())
        {
          throw std::runtime_error{
              "cannot find group name, should not happen!"};
        }
        ret.index_to_ratio.emplace_back(
            std::distance(group_names_rgbs.begin(), group_name_it), w);
        break;
      }
    }
  }
  if (ret.index_to_ratio.empty())
    return std::nullopt;
  percents_to_ratios(ret);
  return ret;
}

auto const& sorted_hands()
{
  using namespace prc::literals;
  // clang-format off
    static std::vector const hands{
    "AA"_re, "AKs"_re, "AQs"_re, "AJs"_re, "ATs"_re, "A9s"_re, "A8s"_re, "A7s"_re, "A6s"_re, "A5s"_re, "A4s"_re, "A3s"_re, "A2s"_re,
    "AKo"_re, "KK"_re, "KQs"_re, "KJs"_re, "KTs"_re, "K9s"_re, "K8s"_re, "K7s"_re, "K6s"_re, "K5s"_re, "K4s"_re, "K3s"_re, "K2s"_re,
    "AQo"_re, "KQo"_re, "QQ"_re, "QJs"_re, "QTs"_re, "Q9s"_re, "Q8s"_re, "Q7s"_re, "Q6s"_re, "Q5s"_re, "Q4s"_re, "Q3s"_re, "Q2s"_re,
    "AJo"_re, "KJo"_re, "QJo"_re, "JJ"_re, "JTs"_re, "J9s"_re, "J8s"_re, "J7s"_re, "J6s"_re, "J5s"_re, "J4s"_re, "J3s"_re, "J2s"_re,
    "ATo"_re, "KTo"_re, "QTo"_re, "JTo"_re, "TT"_re, "T9s"_re, "T8s"_re, "T7s"_re, "T6s"_re, "T5s"_re, "T4s"_re, "T3s"_re, "T2s"_re,
    "A9o"_re, "K9o"_re, "Q9o"_re, "J9o"_re, "T9o"_re, "99"_re, "98s"_re, "97s"_re, "96s"_re, "95s"_re, "94s"_re, "93s"_re, "92s"_re,
    "A8o"_re, "K8o"_re, "Q8o"_re, "J8o"_re, "T8o"_re, "98o"_re, "88"_re, "87s"_re, "86s"_re, "85s"_re, "84s"_re, "83s"_re, "82s"_re,
    "A7o"_re, "K7o"_re, "Q7o"_re, "J7o"_re, "T7o"_re, "97o"_re, "87o"_re, "77"_re, "76s"_re, "75s"_re, "74s"_re, "73s"_re, "72s"_re,
    "A6o"_re, "K6o"_re, "Q6o"_re, "J6o"_re, "T6o"_re, "96o"_re, "86o"_re, "76o"_re, "66"_re, "65s"_re, "64s"_re, "63s"_re, "62s"_re,
    "A5o"_re, "K5o"_re, "Q5o"_re, "J5o"_re, "T5o"_re, "95o"_re, "85o"_re, "75o"_re, "65o"_re, "55"_re, "54s"_re, "53s"_re, "52s"_re,
    "A4o"_re, "K4o"_re, "Q4o"_re, "J4o"_re, "T4o"_re, "94o"_re, "84o"_re, "74o"_re, "64o"_re, "54o"_re, "44"_re, "43s"_re, "42s"_re,
    "A3o"_re, "K3o"_re, "Q3o"_re, "J3o"_re, "T3o"_re, "93o"_re, "83o"_re, "73o"_re, "63o"_re, "53o"_re, "43o"_re, "33"_re, "32s"_re,
    "A2o"_re, "K2o"_re, "Q2o"_re, "J2o"_re, "T2o"_re, "92o"_re, "82o"_re, "72o"_re, "62o"_re, "52o"_re, "42o"_re, "32o"_re, "22"_re,
  };
  // clang-format on
  return hands;
}

std::string to_little_word(std::uint16_t n)
{
  std::string ret(2, '\0');
  std::memcpy(
      ret.data(), reinterpret_cast<char const*>(&n), sizeof(std::uint16_t));
  return ret;
}

std::string to_little_dword(int n)
{
  std::string ret(4, '\0');
  std::memcpy(ret.data(), reinterpret_cast<char const*>(&n), sizeof(int));
  return ret;
}

std::string to_radix_double(double d)
{
  std::string ret(8, '\0');

  std::memcpy(ret.data(), reinterpret_cast<char const*>(&d), sizeof(double));
  return ret;
}

std::string to_utf16_string(std::string const& utf8)
{
  auto ret = to_little_dword(0x65) + '\xff' + '\xfe';
  auto const utf16 = boost::locale::conv::between(utf8, "UTF16-LE", "UTF8");
  if (utf16.size() % 2 != 0)
    throw std::runtime_error("utf16 size must be an even number");
  auto const nb_code_units = utf16.size() / 2;
  if (nb_code_units > 30'000)
    throw std::runtime_error("utf16 size must be <= 60 000");
  ret += "\xff";
  // 255 must be encoded on two bytes, else you get two 0xff which breaks the
  // format
  if (nb_code_units >= 0xff)
  {
    ret += "\xff";
    ret += to_little_word(nb_code_units);
  }
  else
  {
    ret += static_cast<char>(nb_code_units);
  }
  ret += utf16;
  return ret;
}

std::string to_category(prc::folder const& f)
{
  auto ret = to_little_dword(0x00003039);
  if (f.name() == "/")
    ret += to_utf16_string("default_name");
  else
    ret += to_utf16_string(f.name());
  ret += to_little_dword(0);
  ret += static_cast<char>(1) + to_little_dword(0) + to_little_dword(0) +
         to_little_dword(f.entries().size());
  return ret;
}

std::string to_range_content(
    std::vector<prc::range::weighted_elems> const& elems)
{
  std::stringstream ss;
  for (auto const& [w, e] : elems)
  {
    ss << '[' << w << ']';
    for (auto i = 0; i < e.size(); ++i)
    {
      // GTO+ doesn't like when hand ranges are low-high, must be high-low
      if (auto hr = e[i].get_if<prc::hand_range>())
        ss << hr->to() << '-' << hr->from();
      else
        ss << e[i];
      if (i != e.size() - 1)
        ss << ',';
    }
    ss << "[/" << w << "],";
  }
  auto utf8 = ss.str();
  if (!utf8.empty())
    utf8.pop_back();
  return to_utf16_string(utf8);
}

std::string to_group_info(std::vector<prc::range> const& subranges,
                          std::vector<group_name_rgb>& group_names_rgbs)
{
  std::string ret;
  for (auto const& sub : subranges)
  {
    auto const group_it = std::find_if(
        group_names_rgbs.begin(), group_names_rgbs.end(), [&](auto& g) {
          return g.rgb == sub.rgb();
        });
    auto const idx = std::distance(group_names_rgbs.begin(), group_it);
    if (group_it == group_names_rgbs.end())
      group_names_rgbs.push_back({sub.name(), sub.rgb()});
    ret += to_little_dword(0x00003039) + to_utf16_string(sub.name()) +
           to_little_dword(0) + '\x1' + to_little_dword(2) +
           to_little_dword(idx) + to_little_dword(0);
  }
  return ret;
}

std::string to_hand_info(std::vector<prc::range> const& subranges,
                         std::vector<group_name_rgb> const& group_names_rgbs)
{
  std::string ret;
  auto const& all_hands = sorted_hands();
  std::vector<hand_info> info;
  for (auto i = 0; i < all_hands.size(); ++i)
  {
    auto const opt_info =
        get_hand_info(all_hands[i], subranges, group_names_rgbs);
    if (opt_info)
      info.push_back(*opt_info);
    else
      info.push_back(hand_info{{{0, 1.0}}});
  }
  for (auto const& elem : info)
  {
    ret += to_little_dword(0x84) + to_little_dword(elem.index_to_ratio.size());
    for (auto const& [idx, ratio] : elem.index_to_ratio)
      ret += to_little_dword(idx);
    for (auto const& [idx, ratio] : elem.index_to_ratio)
      ret += to_radix_double(ratio);
  }
  return ret;
}

std::string to_range(prc::range const& r,
                     std::vector<group_name_rgb>& group_names_rgbs)
{
  auto ret = to_little_dword(0x00003039);
  ret += to_utf16_string(r.name()) + to_little_dword(0);
  if (r.subranges().empty())
    ret += '\x00';
  else
    ret += '\x01';
  ret += to_little_dword(1) + to_little_dword(0) +
         to_little_dword(r.subranges().size());
  ret += to_group_info(r.subranges(), group_names_rgbs);
  ret += to_range_content(r.elems());
  ret += to_hand_info(r.subranges(), group_names_rgbs);
  ret += to_little_dword(r.subranges().size());
  for (auto const& sub : r.subranges())
  {
    auto const group_it = std::find_if(
        group_names_rgbs.begin(), group_names_rgbs.end(), [&](auto& g) {
          return g.rgb == sub.rgb();
        });
    auto const idx = std::distance(group_names_rgbs.begin(), group_it);
    if (group_it == group_names_rgbs.end())
      throw std::runtime_error{"cannot find group name, should not happen!"};
    ret += to_little_dword(idx);
  }
  return ret;
}

std::string serialize_impl(prc::folder const& parent_folder,
                           std::vector<group_name_rgb>& group_names_rgbs)
{
  auto ret = to_category(parent_folder);
  for (auto const& entry : parent_folder.entries())
  {
    if (auto f = boost::variant2::get_if<prc::folder>(&entry))
      ret += serialize_impl(*f, group_names_rgbs);
    else
      ret +=
          to_range(boost::variant2::get<prc::range>(entry), group_names_rgbs);
  }
  return ret;
}

std::string serialize_settings(
    std::vector<group_name_rgb> const& group_names_rgbs)
{
  using namespace std::string_literals;
  auto ret = R"-([ACTION COLORS]
1) 225 68 68
2) 114 191 68
3) 0 130 202
4) 225 0 225
5) 0 225 225
6) 255 183 71
7) 0 191 255
8) 255 105 180

[HEATMAP COLORS]
1) 255 0 0
2) 255 230 0
3) 0 200 0

[PREFLOP GROUP COLORS]
)-"s;

  for (auto i = 0; i < group_names_rgbs.size(); ++i)
  {
    ret += std::to_string(i + 1) + ") " +
           rgb_to_string(group_names_rgbs[i].rgb) + '\n';
  }
  return ret;
}
}

serialized_content serialize(prc::folder const& f)
{
  serialized_content ret;
  std::vector<group_name_rgb> group_names_rgbs;
  ret.newdefs3 = serialize_impl(f, group_names_rgbs);
  ret.settings = serialize_settings(group_names_rgbs);
  for (auto const& elem : group_names_rgbs)
  {
    std::cout << elem.name << ' ' << std::hex << elem.rgb << std::dec
              << std::endl;
  }
  return ret;
}
}
