#pragma once

#include <filesystem>

#include <prc/folder.hpp>

namespace prc::equilab
{
prc::folder parse(std::filesystem::path const&);
}
