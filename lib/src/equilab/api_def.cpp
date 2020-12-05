#include <prc/equilab/parser/api_def.hpp>
#include <prc/equilab/parser/config.hpp>

namespace prc::equilab::parser
{
BOOST_SPIRIT_INSTANTIATE(group_info_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(weighted_hands_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(group_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(range_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(folder_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);
BOOST_SPIRIT_INSTANTIATE(file_type,
                         std::u32string::const_iterator,
                         context_type<std::u32string::const_iterator>);

file_type file()
{
  return _file;
}

range_type range()
{
  return _range;
}
}
