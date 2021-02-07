#pragma once

#include <string>
#include <string_view>

namespace prc::detail
{
std::string utf32_to_utf8(std::u32string_view);
std::u16string utf32_to_utf16le(std::u32string_view);
std::u32string utf16le_to_utf32(std::u16string_view);
std::u32string utf8_to_utf32(std::string_view);
std::u16string utf8_to_utf16le(std::string_view);
std::string utf16le_to_utf8(std::u16string_view);
}
