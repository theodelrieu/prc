#include <prc/detail/unicode.hpp>

#include <unicode/unistr.h>

#include <stdexcept>

using namespace icu;

namespace prc::detail
{
std::string utf32_to_utf8(std::u32string_view sv)
{
  auto ustr = UnicodeString::fromUTF32(
      reinterpret_cast<int32_t const*>(sv.data()), sv.size());
  std::string ret;
  ustr.toUTF8String(ret);
  return ret;
}

std::u16string utf32_to_utf16le(std::u32string_view sv)
{
  auto ustr = UnicodeString::fromUTF32(
      reinterpret_cast<int32_t const*>(sv.data()), sv.size());
  return std::u16string(ustr.getTerminatedBuffer());
}

std::u32string utf16le_to_utf32(std::u16string_view sv)
{
  UnicodeString ustr(sv.data(), sv.size());
  std::u32string ret(sv.size() * 3, 0);
  UErrorCode err{};
  auto s =
      ustr.toUTF32(reinterpret_cast<UChar32*>(ret.data()), ret.size(), err);
  if (U_FAILURE(err))
    throw std::runtime_error{"failed to convert UTF16-LE to UTF32"};
  ret.resize(s);
  return ret;
}

std::u32string utf8_to_utf32(std::string_view sv)
{
  auto ustr = UnicodeString::fromUTF8(StringPiece(sv.data(), sv.size()));

  std::u32string ret(sv.size() * 4, 0);
  UErrorCode err{};
  auto s =
      ustr.toUTF32(reinterpret_cast<UChar32*>(ret.data()), ret.size(), err);
  if (U_FAILURE(err))
    throw std::runtime_error{"failed to convert UTF16-LE to UTF32"};
  ret.resize(s);
  return ret;
}

std::u16string utf8_to_utf16le(std::string_view sv)
{
  auto ustr = UnicodeString::fromUTF8(StringPiece(sv.data(), sv.size()));
  return std::u16string(ustr.getTerminatedBuffer());
}

std::string utf16le_to_utf8(std::u16string_view sv)
{
  UnicodeString ustr(sv.data(), sv.size());
  std::string ret;
  ustr.toUTF8String(ret);
  return ret;
}
}
