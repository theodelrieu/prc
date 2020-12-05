#include <prc/folder.hpp>

#include <iomanip>
#include <iostream>

namespace prc
{
namespace
{
auto const get_depth = [](auto const& e) { return e.depth; };
auto const get_name = [](auto const& e) { return e.name(); };

template <typename I, typename S>
void recurse_entries(I& current,
                     S end,
                     folder& parent_folder,
                     int current_depth)
{
  while (current != end)
  {
    auto const depth = boost::apply_visitor(get_depth, *current);

    if (depth != current_depth)
      return;
    if (auto f = boost::get<equilab::parser::ast::folder>(&(*current)))
    {
      folder new_folder{f->name};

      if (++current != end)
      {
        auto const subfolder_depth = boost::apply_visitor(get_depth, *current);
        if (subfolder_depth > depth)
          recurse_entries(current, end, new_folder, subfolder_depth);
      }
      parent_folder.add_entry(new_folder);
    }
    else
    {
      auto& r = boost::get<equilab::parser::ast::range>(*current);
      parent_folder.add_entry(prc::range(r));
      ++current;
    }
  }
}
void print_subranges(std::ostream& os,
                     std::vector<range> const& subranges,
                     int depth)
{
  for (auto const& sub : subranges)
  {
    os << "\n"
       << std::string(depth, '\t') << "Subrange: " << sub.name() << ' '
       << std::hex << sub.rgb() << std::dec << ' ';
    for (auto const& elem : sub.elems())
      os << elem;
    print_subranges(os, sub.subranges(), depth + 1);
  }
}
}

folder::folder(std::string name) : _name(std::move(name))
{
}

folder::folder(std::string name,
               std::vector<equilab::parser::ast::entry> const& entries)
  : folder(std::move(name))
{
  auto b = entries.begin();
  recurse_entries(b, entries.end(), *this, 1);
}

void folder::add_entry(folder const& f)
{
  _entries.push_back(f);
}

void folder::add_entry(range const& r)
{
  _entries.push_back(r);
}

void folder::remove_entry(std::string const& name)
{
  _entries.erase(std::remove_if(_entries.begin(),
                                _entries.end(),
                                [&name](auto& e) {
                                  return boost::variant2::visit(get_name, e) ==
                                         name;
                                }),
                 _entries.end());
}

std::string const& folder::name() const
{
  return _name;
}

auto folder::entries() const -> std::vector<entry> const&
{
  return _entries;
}

auto folder::entries() -> std::vector<entry>&
{
  return _entries;
}

bool operator==(folder const& lhs, folder const& rhs)
{
  return std::tie(lhs.name(), lhs.entries()) ==
         std::tie(rhs.name(), rhs.entries());
}

bool operator!=(folder const& lhs, folder const& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, folder const& f)
{
  os << f.name() << ": {";
  for (auto const& elem : f.entries())
  {
    if (auto p = boost::variant2::get_if<range>(&elem))
    {
      os << p->name() << ' ' << p->rgb() << ' ';
      for (auto const& elem : p->elems())
        os << elem;
      print_subranges(os, p->subranges(), 0);
    }
    else
    {
      os << boost::variant2::get<folder>(elem);
    }
  }
  os << "}";
  return os;
}
}
