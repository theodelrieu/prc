#include <prc/gtoplus/parser/api_def.hpp>
#include <prc/gtoplus/parser/config.hpp>
#include <prc/parser/api.hpp>

#include <ostream>

namespace prc::gtoplus::parser
{
BOOST_SPIRIT_INSTANTIATE(file_type,
                         std::string::const_iterator,
                         context_type<std::string::const_iterator>);

file_type file()
{
  return _file_type;
}

namespace ast
{
std::ostream& operator<<(std::ostream& os, entry_type e)
{
  switch (e)
  {
  case entry_type::category:
    os << "category";
    break;
  case entry_type::range:
    os << "range";
    break;
  case entry_type::group:
    os << "group";
    break;
  case entry_type::grouped_range:
    os << "grouped_range";
    break;
  }
  return os;
}
}
}
