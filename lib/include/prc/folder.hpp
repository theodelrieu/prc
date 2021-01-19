#pragma once

#include <prc/equilab/parser/ast.hpp>
#include <prc/pio/parser/ast.hpp>
#include <prc/range.hpp>

#include <boost/variant2/variant.hpp>

#include <filesystem>
#include <iosfwd>
#include <vector>

namespace prc
{
class folder
{
public:
  using entry = boost::variant2::variant<folder, range>;

  folder() = default;
  explicit folder(std::string name);
  folder(std::string name, std::vector<equilab::parser::ast::entry> const&);

  void add_entry(folder const&);
  void add_entry(range const&);
  void remove_entry(std::string const& name);
  void set_name(std::string);

  std::string const& name() const;
  std::vector<entry> const& entries() const;

  std::vector<entry>& entries();

private:
  std::string _name;
  std::vector<entry> _entries;
};

bool operator==(folder const& lhs, folder const& rhs);
bool operator!=(folder const& lhs, folder const& rhs);
std::ostream& operator<<(std::ostream&, folder const&);
}
