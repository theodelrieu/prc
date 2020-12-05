#pragma once

#include <iosfwd>
#include <string>

#include <prc/parser/ast.hpp>
#include <prc/rank.hpp>
#include <prc/suit.hpp>

namespace prc
{
class card
{
public:
  card() = default;
  card(prc::rank r, prc::suit s);
  explicit card(parser::ast::card);

  prc::rank const& rank() const;
  prc::suit const& suit() const;

  std::string string() const;

private:
  prc::rank _r;
  prc::suit _s;
};

bool operator==(card const&, card const&) noexcept;
bool operator!=(card const&, card const&) noexcept;
bool operator<(card const&, card const&) noexcept;

std::ostream& operator<<(std::ostream&, card const&);
}
