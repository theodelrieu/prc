#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/fusion/include/io.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/variant.hpp>

#include <prc/parser/ast.hpp>

namespace prc::equilab::parser::ast
{
struct group_info
{
  int index;
  int parent_index;
  int rgb;
  std::string name;
  int nesting_index;
};

struct weighted_hands
{
  double weight;
  std::vector<prc::parser::ast::range_elem> hands;
};

struct group
{
  std::vector<ast::weighted_hands> weighted_hands;
  std::optional<ast::group_info> info;
};

struct range
{
  int depth;
  std::string name;
  std::vector<group> groups;
  std::optional<std::string> note;
};

struct folder
{
  int depth;
  std::string name;
};

using entry = boost::variant<range, folder>;

using boost::fusion::operator<<;
}
