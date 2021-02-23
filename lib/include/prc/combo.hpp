#pragma once

#include <prc/card.hpp>
#include <prc/parser/ast.hpp>
#include <prc/rank.hpp>
#include <prc/suit.hpp>

#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

namespace prc
{
class range_elem;
class hand;

class combo
{
public:
  combo() = default;
  combo(card, card);
  explicit combo(parser::ast::combo const&);

  card const& high() const;
  card const& low() const;

  std::string string() const;

  bool suited() const;
  bool offsuit() const;
  bool paired() const;

private:
  card _high;
  card _low;
};

struct weighted_combo
{
  prc::combo combo;
  double weight;
};

bool operator==(combo const&, combo const&) noexcept;
bool operator!=(combo const&, combo const&) noexcept;
bool operator<(combo const&, combo const&) noexcept;

std::ostream& operator<<(std::ostream&, combo const&);

std::vector<combo> expand_combos(range_elem const&);
std::vector<combo> expand_combos(std::vector<range_elem> const&);
std::vector<hand> expand_hands(std::vector<range_elem> const&);

std::vector<range_elem> reduce_combos(std::vector<combo> const&);

std::vector<range_elem> const& any_two();
std::vector<prc::combo> const& any_two_combos();

std::vector<prc::combo> generate_all_combos();
std::vector<weighted_combo> weigh_combos(std::vector<combo> const& all_combos,
                                         std::vector<double> const& weights);

std::vector<weighted_combo> filter_folded_combos(
    std::vector<weighted_combo> weighted_combos);

inline namespace literals
{
combo operator"" _c(char const*, std::size_t);
}
}
