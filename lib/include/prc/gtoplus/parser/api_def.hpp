#pragma once

#define BOOST_SPIRIT_X3_DEBUG
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/locale.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>
#include <boost/variant.hpp>

#include <prc/gtoplus/parser/api.hpp>
#include <prc/gtoplus/parser/ast.hpp>
#include <prc/gtoplus/parser/ast_adapted.hpp>
#include <prc/gtoplus/parser/config.hpp>
#include <prc/parser/api.hpp>
#include <prc/parser/as_type.hpp>
#include <prc/parser/ast.hpp>

#include <cstring>

namespace prc::gtoplus::parser
{
struct error_handler
{
  template <typename Iterator, typename Exception, typename Context>
  x3::error_handler_result on_error(Iterator& first,
                                    Iterator const& last,
                                    Exception const& x,
                                    Context const& context)
  {
    using namespace std::string_literals;

    auto& error_handler = x3::get<x3::error_handler_tag>(context);
    auto msg = "Expecting "s + x.which() + " here:";
    error_handler(x.where(), msg);
    return x3::error_handler_result::fail;
  }
};

// it's written in this order in the file, not a big-endian number but using
// big_word allows to write bytes in the same order
auto const separator = x3::omit[x3::big_dword(0x39300000)];

struct utf16_to_utf8_string_parser : x3::parser<utf16_to_utf8_string_parser>
{
  using attribute_type = std::string;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    if (!x3::parse(it, last, x3::big_dword(0x65000000) >> x3::big_word(0xfffe)))
    {
      return false;
    }
    std::vector<char> buf;
    if (!x3::parse(it,
                   last,
                   // max gto+ size is 30 000, so max 2 bytes
                   x3::repeat(1, 2)[x3::char_(0xff)],
                   buf))
    {
      return false;
    }
    auto const encoded_size_bytes = buf.size();
    buf.clear();
    if (!x3::parse(it, last, x3::repeat(encoded_size_bytes)[x3::char_], buf))
      return false;
    auto nb_utf16_code_units = 0u;
    std::memcpy(&nb_utf16_code_units, buf.data(), buf.size());
    std::vector<char16_t> utf16_buf;
    if (!x3::parse(it,
                   last,
                   x3::repeat(nb_utf16_code_units)[x3::little_word],
                   utf16_buf))
      return false;
    attr = boost::locale::conv::utf_to_utf<char>(
        utf16_buf.data(), utf16_buf.data() + utf16_buf.size());
    first = it;
    return true;
  }
};

inline constexpr utf16_to_utf8_string_parser utf16_to_utf8_string;

struct info_parser : x3::parser<info_parser>
{
  using attribute_type = ast::info;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    std::string name;
    if (!x3::parse(it, last, separator >> utf16_to_utf8_string, name))
      return false;
    if (!x3::parse(it, last, x3::little_dword(0x00)))
      return false;
    std::pair<unsigned char, unsigned int> p;
    if (!x3::parse(it, last, x3::byte_ >> x3::little_dword, p))
      return false;
    ast::entry_type e;
    auto [is_category, is_range] = p;
    if (is_category > 1 || is_range > 2)
      return false;
    else if (is_category == 0)
    {
      if (is_range == 1)
        e = ast::entry_type::grouped_range;
      else
        return false;
    }
    else if (is_range == 1)
      e = ast::entry_type::range;
    else
      e = ast::entry_type::category;
    int nb_subentries;
    if (!x3::parse(it,
                   last,
                   x3::little_dword(0x00) >> x3::little_dword,
                   nb_subentries))
    {
      return false;
    }
    first = it;
    attr = ast::info{std::move(name), e, nb_subentries};
    return true;
  }
};

inline constexpr info_parser _info;

struct category_parser : x3::parser<category_parser>
{
  using attribute_type = ast::category;

  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    ast::info info;
    if (!x3::parse(it, last, _info, info))
      return false;
    if (info.type != ast::entry_type::category)
      return false;
    attr = ast::category{std::move(info)};
    first = it;
    return true;
  }
};

inline constexpr category_parser _category;

struct range_parser : x3::parser<range_parser>
{
  using attribute_type = ast::range;
  template <typename Iterator,
            typename Context,
            typename RuleContext,
            typename Attribute>
  bool parse(Iterator& first,
             Iterator const& last,
             Context const& context,
             RuleContext const& rule_context,
             Attribute& attr) const
  {
    auto it = first;
    ast::info info;
    if (!x3::parse(it, last, _info, info))
      return false;
    if (info.type != ast::entry_type::range || info.nb_subentries != 0)
      return false;
    std::string range_content;
    if (!x3::parse(it, last, utf16_to_utf8_string, range_content))
      return false;
    // TODO handle weights
    // this horror seems needed, haven't found a better way of reusing the
    // context
    std::vector<prc::parser::ast::range_elem> elems;
    if (!x3::phrase_parse(
            range_content.cbegin(),
            range_content.cend(),
            x3::with<x3::error_handler_tag>(context.get(
                boost::mpl::identity<
                    x3::error_handler_tag>{}))[prc::parser::range_elem() % ','],
            x3::space,
            elems))
    {
      return false;
    }
    // TODO handle both range / grouped range in same parser
    if (!x3::parse(it,
                   last,
                   x3::repeat(169)[x3::little_dword(0x84) >>
                                   x3::repeat(4)[x3::little_dword]] >>
                       x3::little_dword(0x00)))
    {
      return false;
    }
    first = it;
    attr = ast::range{std::move(info), std::move(elems)};
    return true;
  }
};

inline constexpr range_parser _range;

file_type const _file_type = "file";
auto const _file_type_def = x3::no_skip[+(_category | _range) > x3::eoi];

BOOST_SPIRIT_DEFINE(_file_type);

struct file_class : error_handler
{
};
}
