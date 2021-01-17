#pragma once

#include <string>

#include <prc/folder.hpp>

namespace prc::gtoplus
{
struct serialized_content
{
  std::string newdefs3;
  std::string settings;
};

serialized_content serialize(prc::folder const&);
}
