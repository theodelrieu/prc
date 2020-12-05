#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <prc/combo.hpp>
#include <prc/range.hpp>

namespace prc::pio
{
range parse(std::filesystem::path const& pio_range_path);
}
