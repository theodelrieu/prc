#pragma once

#include <prc/parser/api.hpp>
#include <prc/rank.hpp>
#include <prc/suitedness.hpp>

#include <iosfwd>
#include <string>

namespace prc
{
class unpaired_hand
{
public:
  unpaired_hand() = default;
  unpaired_hand(rank, rank, suitedness);
  explicit unpaired_hand(parser::ast::unpaired_hand const&);

  rank high() const;
  rank low() const;

  bool suited() const;
  bool offsuit() const;

  std::string string() const;

private:
  rank _high;
  rank _low;
  suitedness _suitedness;
};

bool operator==(unpaired_hand const&, unpaired_hand const&) noexcept;
bool operator!=(unpaired_hand const&, unpaired_hand const&) noexcept;

bool operator<(unpaired_hand const&, unpaired_hand const&) noexcept;

std::ostream& operator<<(std::ostream&, unpaired_hand const&);
}
