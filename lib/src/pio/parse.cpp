#include <prc/pio/parse.hpp>

#include <prc/detail/unicode.hpp>
#include <prc/pio/parser/api.hpp>

#include <boost/spirit/home/x3.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;

namespace prc::pio
{
namespace
{
std::u32string read_all(fs::path const& p)
{
  if (!fs::exists(p))
    throw std::runtime_error("No such path: " + p.string());
  if (p.extension() != ".txt")
    throw std::runtime_error("invalid pio range path: " + p.string());
  std::ifstream ifs{p};
  std::string content(std::istreambuf_iterator<char>(ifs), {});
  return detail::utf8_to_utf32(content);
}

template <typename I, typename S>
void recurse_entries(I& current,
                     S end,
                     folder& parent_folder,
                     fs::path const& parent_absolute_path)
{
  while (current != end)
  {
    auto const& current_path = *current;

    if (current_path.parent_path() != parent_absolute_path)
      return;
    if (is_directory(current_path))
    {
      folder new_folder{current_path.filename().string()};

      if (++current != end)
      {
        auto const& subfolder_path = *current;
        if (subfolder_path.parent_path() == current_path)
          recurse_entries(current, end, new_folder, current_path);
      }
      if (!new_folder.entries().empty())
        parent_folder.add_entry(std::move(new_folder));
    }
    else
    {
      if (current_path.extension() == ".txt")
      {
        // TODO report error through a callback that could be used to show
        // progress bar?
        try
        {
          auto range = parse_range(current_path);
          range.set_name(current_path.stem());
          parent_folder.add_entry(std::move(range));
        }
        catch (std::exception const& e)
        {
          std::cerr << "An exception occurred while parsing " << current_path
                    << ": " << e.what() << std::endl;
        }
      }
      ++current;
    }
  }
}
}

prc::range parse_range(fs::path const& pio_range_path)
{
  auto const content = read_all(pio_range_path);

  x3::error_handler<std::u32string::const_iterator> error_handler(
      content.begin(), content.end(), std::cerr);

  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[x3::expect[parser::range()] > x3::eoi];
  parser::ast::range r;
  x3::phrase_parse(content.begin(), content.end(), ctx, x3::unicode::space, r);
  return prc::range{r};
}

folder parse_folder(fs::path const& pio_folder_path)
{
  prc::folder root{"/"};
  std::vector<fs::path> paths;

  for (auto& p : fs::recursive_directory_iterator{pio_folder_path})
    paths.push_back(p.path());
  if (paths.empty())
    return root;
  std::sort(paths.begin(), paths.end());
  auto begin = paths.begin();
  auto const end = paths.end();
  recurse_entries(begin, end, root, pio_folder_path);
  return root;
}
}
