#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include <boost/fusion/include/io.hpp>
#include <boost/variant2/variant.hpp>

#include <prc/parser/ast.hpp>

namespace prc::gtoplus::parser::ast
{
enum class entry_type
{
  category,
  range,
  group,
  grouped_range
};

struct info
{
  std::string name;
  entry_type type;
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

struct range
{
  ast::info info;
  std::vector<ast::weighted_elems> weighted_elems;
};

using entry = boost::variant<category, range>;

using boost::fusion::operator<<;

std::ostream& operator<<(std::ostream&, entry_type);
}
