#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include <boost/variant2/variant.hpp>

#include <prc/parser/ast.hpp>

namespace prc::gtoplus::parser::ast
{
enum class entry_type
{
  category,
  range,
  group,
};

struct info
{
  std::string name;
  entry_type type;
  int group_index;
  int nb_subentries;
};

struct category
{
  ast::info info;
};

struct weighted_elems
{
  double weight;
  std::vector<prc::parser::ast::range_elem> elems;
};

struct group_ratio
{
  int index;
  double ratio;
};

struct hand_info
{
  prc::parser::ast::hand hand;
  std::vector<group_ratio> group_ratios;
};

struct range
{
  ast::info info;
  std::vector<ast::info> groups;
  std::vector<ast::weighted_elems> weighted_elems;
  std::vector<ast::hand_info> hand_info;
};

using entry = boost::variant<category, range>;

using boost::fusion::operator<<;

std::ostream& operator<<(std::ostream&, entry_type);
}
