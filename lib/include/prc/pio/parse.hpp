#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <prc/combo.hpp>
#include <prc/folder.hpp>
#include <prc/range.hpp>

namespace prc::pio
{
range parse_range(std::filesystem::path const& pio_range_path);
folder parse_folder(std::filesystem::path const& pio_folder_path);
}
