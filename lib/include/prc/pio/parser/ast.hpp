#pragma once

#include <string>
#include <vector>

#include <boost/fusion/include/io.hpp>

namespace prc::pio::parser::ast
{
struct base_range
{
  std::string name;
  std::vector<double> weights;
};

struct subrange
{
  bool included;
  int rgb;
  std::string name;
  std::vector<double> weights;
};

struct range
{
  ast::base_range base_range;
  std::vector<subrange> subranges;
};

using boost::fusion::operator<<;
}
