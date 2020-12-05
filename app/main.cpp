#include <filesystem>
#include <fstream>
#include <iostream>

#include <lyra/lyra.hpp>

#include <prc/equilab/parser/api.hpp>
#include <prc/equilab/serialize.hpp>
#include <prc/folder.hpp>
#include <prc/pio/parse.hpp>

#include <boost/locale.hpp>
#include <boost/spirit/home/x3.hpp>

#include "actions.hpp"

namespace fs = std::filesystem;
namespace x3 = boost::spirit::x3;

using namespace std::string_literals;

using namespace prc;

namespace
{
template <typename Callable>
void apply_to_subranges(range& r, fs::path const& abs_parent_path, Callable&& c)
{
  std::forward<Callable>(c)(r, abs_parent_path);
  for (auto& sub : r.subranges())
    apply_to_subranges(
        sub, abs_parent_path / r.name(), std::forward<Callable>(c));
}

template <typename Callable>
void apply_to_ranges_impl(folder& current_folder,
                          fs::path const& current_path,
                          bool recurse_subranges,
                          Callable&& c)
{
  for (auto& e : current_folder.entries())
  {
    if (auto r = boost::variant2::get_if<range>(&e))
    {
      if (recurse_subranges)
        apply_to_subranges(*r, current_path, std::forward<Callable>(c));
      else
        c(*r, current_path / current_folder.name());
    }
    else
    {
      apply_to_ranges_impl(boost::variant2::get<folder>(e),
                           current_path / current_folder.name(),
                           recurse_subranges,
                           std::forward<Callable>(c));
    }
  }
}

template <typename Callable>
void apply_to_folders_impl(folder& current_folder,
                           fs::path const& current_path,
                           Callable&& c)
{
  for (auto& e : current_folder.entries())
  {
    if (auto f = boost::variant2::get_if<folder>(&e))
    {
      if (c(*f, current_path / f->name()))
      {
        apply_to_folders_impl(
            *f, current_path / f->name(), std::forward<Callable>(c));
      }
    }
  }
}

void apply_to_ranges(
    folder& root,
    bool recurse_subranges,
    std::vector<std::function<void(range&, fs::path const&)>> const& funcs)
{
  apply_to_ranges_impl(root,
                       fs::path{"/"} / root.name(),
                       recurse_subranges,
                       [&](auto& r, auto const& p) {
                         for (auto const& f : funcs)
                           f(r, p);
                       });
}

void apply_to_folders(folder& root,
                      std::function<bool(folder&, fs::path const&)> const& func)
{
  auto ld = [&](auto& f, auto const& p) { return func(f, p); };
  if (ld(root, root.name()))
    apply_to_folders_impl(root, root.name(), ld);
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
      {
        std::cout << "Adding folder: " << current_path << std::endl;
        parent_folder.add_entry(std::move(new_folder));
      }
    }
    else
    {
      if (current_path.extension() != ".txt")
        std::cout << "Skipping non-pio file: " << current_path << std::endl;
      else
      {
        std::cout << "Adding range: " << current_path << std::endl;
        auto range = pio::parse(current_path);
        range.set_name(current_path.stem());
        parent_folder.add_entry(std::move(range));
      }
      ++current;
    }
  }
}

void serialize_to_equilab(folder const& root, fs::path const& dst)
{
  fs::create_directories(dst.parent_path());
  auto const equilab_content = equilab::serialize(root);
  std::ofstream ofs{dst.string(), std::ios::binary | std::ios::trunc};
  ofs.write(equilab_content.data(), equilab_content.size());
  std::cout << "Wrote " << dst << std::endl;
}

folder load_pio_folder(fs::path const& src)
{
  std::vector<fs::path> paths;
  for (auto& p : fs::recursive_directory_iterator{src})
    paths.push_back(p.path());
  if (paths.empty())
    throw std::runtime_error{"empty src directory: " + src.string()};
  std::sort(paths.begin(), paths.end());

  prc::folder root{"/"};
  auto begin = paths.begin();
  auto const end = paths.end();
  recurse_entries(begin, end, root, src);
  return root;
}

// TODO add equilab/parse.hpp
folder load_equilab_file(fs::path const& src_file)
{
  namespace x3 = boost::spirit::x3;

  std::ifstream ifs(src_file.string(), std::ios::binary);
  std::string const utf16(std::istreambuf_iterator<char>(ifs), {});

  auto const utf8 = boost::locale::conv::between(utf16, "UTF8", "UTF16-LE");
  auto const utf32 = boost::locale::conv::utf_to_utf<char32_t>(utf8);
  x3::error_handler<std::u32string::const_iterator> error_handler(
      utf32.begin(), utf32.end(), std::cerr);

  auto ctx = x3::with<x3::error_handler_tag>(
      std::move(error_handler))[equilab::parser::file()];

  std::vector<equilab::parser::ast::entry> entries;
  auto b = utf32.begin();
  auto e = utf32.end();
  auto r = x3::phrase_parse(b, e, ctx, x3::unicode::space, entries);
  if (!r)
    throw std::runtime_error("failed to parse");
  if (b != e)
    std::cout << "trailing input! " << std::distance(b, e) << " bytes left"
              << std::endl;
  std::cout << entries.size() << " entries parsed" << std::endl;
  return prc::folder{"/", entries};
}

void apply_pio_actions(folder& root)
{
  apply_to_folders(root, actions::remove_empty_ranges());
  apply_to_ranges(root,
                  true,
                  {actions::replace_in_name("FOLD", "Fold"),
                   actions::replace_in_name("__", "%_"),
                   actions::replace_in_name("POT", ""),
                   actions::change_color("Fold", 0xff6da2c0),
                   actions::move_subrange_at_end("Fold")});
  // apply_to_folders(root, actions::fix_parent_ranges());
  apply_to_folders(root, actions::nest_parent_ranges());
  apply_to_ranges(
      root, false, {actions::set_unassigned_to_subrange("Fold", 0xff6da2c0)});
}

void apply_equilab_actions(folder& root)
{
  apply_to_folders(root, actions::nest_parent_ranges());
}
}

int main(int argc, char const* argv[])
{
  auto show_help = false;
  std::string src;
  std::string src_format;
  std::string dst_format;
  std::string dst;

  auto cli = lyra::help(show_help) | lyra::opt(src, "src")["--src"].required() |
             lyra::opt(src_format, "src format")["--src-format"].required() |
             lyra::opt(dst_format, "dst format")["--dst-format"].required() |
             lyra::opt(dst, "dst")["--dst"].required();
  if (auto res = cli.parse({argc, argv}); !res)
  {
    std::cout << "Error in command line: " << res.errorMessage() << std::endl;
    return -1;
  }
  if (show_help)
  {
    std::cout << cli << std::endl;
    return 0;
  }
  // TODO repl
  folder root;
  if (src_format == "pio")
  {
    if (!fs::is_directory(src))
    {
      std::cout << "--src must point to a directory when --src-format=pio"
                << std::endl;
      return -1;
    }
    root = load_pio_folder(fs::absolute(fs::path{src}));
    apply_pio_actions(root);
  }
  else if (src_format == "equilab")
  {
    if (!fs::is_regular_file(src))
    {
      std::cout << "--src must point to a file when --src-format=equilab"
                << std::endl;
      return -1;
    }
    root = load_equilab_file(fs::absolute(fs::path{src}));
    apply_equilab_actions(root);
  }

  if (dst_format == "equilab")
  {
    serialize_to_equilab(root, fs::absolute(fs::path{dst}));
  }
}
