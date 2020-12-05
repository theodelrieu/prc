#pragma once

#include <prc/parser/api.hpp>
#include <prc/rank.hpp>

#include <iosfwd>
#include <string>

namespace prc
{
class paired_hand
{
public:
  paired_hand() = default;
  paired_hand(prc::rank);
  explicit paired_hand(parser::ast::paired_hand);

  prc::rank rank() const;

  std::string string() const;

private:
  prc::rank _r;
};

bool operator==(paired_hand const&, paired_hand const&) noexcept;
bool operator!=(paired_hand const&, paired_hand const&) noexcept;

bool operator<(paired_hand const&, paired_hand const&) noexcept;

std::ostream& operator<<(std::ostream&, paired_hand const&);
}
