#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include <boost/fusion/include/io.hpp>

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

using boost::fusion::operator<<;

std::ostream& operator<<(std::ostream&, entry_type);
}
