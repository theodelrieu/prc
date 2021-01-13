#include <prc/gtoplus/parser/api_def.hpp>
#include <prc/gtoplus/parser/config.hpp>

#include <ostream>

namespace prc::gtoplus::parser
{
BOOST_SPIRIT_INSTANTIATE(info_type,
                         std::string::const_iterator,
                         context_type<std::string::const_iterator>);

info_type info()
{
  return _info;
}

namespace ast
{
std::ostream& operator<<(std::ostream& os, entry_type e)
{
  switch (e)
  {
  case entry_type::category:
    os << "category";
  case entry_type::range:
    os << "range";
  case entry_type::group:
    os << "group";
  case entry_type::grouped_range:
    os << "grouped_range";
  }
  return os;
}
}
}
