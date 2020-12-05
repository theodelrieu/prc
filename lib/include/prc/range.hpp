#pragma once

#include <iosfwd>
#include <string>

#include <prc/combo.hpp>
#include <prc/range_elem.hpp>

#include <prc/equilab/parser/ast.hpp>
#include <prc/pio/parser/ast.hpp>

namespace prc
{
class range
{
public:
  struct weighted_elems
  {
    double weight;
    std::vector<range_elem> elems;
  };

  range() = default;
  range(std::string name,
        std::vector<weighted_elems> elems,
        int rgb = 0,
        std::vector<range> subranges = {});

  explicit range(equilab::parser::ast::range const&);
  explicit range(pio::parser::ast::range const&);

  void add_subrange(range const&);
  void add_subrange(range&&);

  range* find_subrange(std::string const& name);
  range const* find_subrange(std::string const& name) const;

  void set_rgb(int);
  void set_name(std::string name);
  void set_elems(std::vector<weighted_elems> elems);

  std::string const& name() const;
  int rgb() const;
  std::vector<weighted_elems> const& elems() const;
  std::vector<range> const& subranges() const;
  std::vector<range>& subranges();

private:
  std::string _name;
  std::vector<weighted_elems> _elems;
  std::vector<range> _subranges;
  int _rgb;
};

bool operator==(range const& lhs, range const& rhs);
bool operator!=(range const& lhs, range const& rhs);

bool operator==(range::weighted_elems const& lhs,
                range::weighted_elems const& rhs);
bool operator!=(range::weighted_elems const& lhs,
                range::weighted_elems const& rhs);

std::vector<range::weighted_elems> unassigned_elems(range const& r);
// TODO test
std::vector<range::weighted_elems> adjust_weights(
    std::vector<range::weighted_elems> const& base_range_elems, range const& r);
std::ostream& operator<<(std::ostream&, range::weighted_elems const&);
}
