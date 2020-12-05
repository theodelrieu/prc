#include <prc/pio/parser/api_def.hpp>
#include <prc/pio/parser/config.hpp>

namespace prc::pio::parser
{
BOOST_SPIRIT_INSTANTIATE(base_range_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(subrange_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(range_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);

base_range_type base_range()
{
  return _base_range;
}

subrange_type subrange()
{
  return _subrange;
}

range_type range()
{
  return _range;
}
}
