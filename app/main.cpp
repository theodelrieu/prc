#include <filesystem>
#include <fstream>
#include <iostream>

#include <lyra/lyra.hpp>

#include <prc/equilab/parse.hpp>
#include <prc/equilab/serialize.hpp>
#include <prc/folder.hpp>
#include <prc/gtoplus/serialize.hpp>
#include <prc/pio/parse.hpp>
#include <prc/pio/serialize.hpp>

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

void serialize_to_equilab(folder const& root, fs::path const& dst)
{
  fs::create_directories(dst.parent_path());
  auto const equilab_content = equilab::serialize(root);
  std::ofstream ofs{dst.string(), std::ios::binary | std::ios::trunc};
  ofs.write(reinterpret_cast<char const*>(equilab_content.data()),
            2 * equilab_content.size());
  std::cout << "Wrote " << dst << std::endl;
}

void serialize_to_gtoplus(folder const& root, fs::path const& dst)
{
  fs::create_directories(dst);
  auto const [newdefs, settings] = gtoplus::serialize(root);
  std::ofstream newdefs_stream{(dst / "newdefs3.txt").string(),
                               std::ios::binary | std::ios::trunc};
  newdefs_stream.write(newdefs.data(), newdefs.size());
  std::cout << "Wrote " << dst / "newdefs3.txt" << std::endl;
  std::ofstream settings_stream((dst / "settings.txt").string(),
                                std::ios::trunc);
  settings_stream.write(settings.data(), settings.size());
  std::cout << "Wrote " << dst / "settings.txt" << std::endl;
}

void serialize_to_pio_impl(folder const& current_folder,
                           fs::path const& current_abs_path)
{
  for (auto& e : current_folder.entries())
  {
    if (auto r = boost::variant2::get_if<range>(&e))
    {
      auto const filename = current_abs_path / (r->name() + ".txt"s);
      std::ofstream ofs{filename.string(), std::ios::trunc};
      auto const content = pio::serialize(*r);
      ofs.write(content.data(), content.size());
      std::cout << "Wrote " << filename << std::endl;
    }
    else
    {
      auto const& subfolder = boost::variant2::get<folder>(e);
      auto const subfolder_abs_path = current_abs_path / subfolder.name();
      fs::create_directories(subfolder_abs_path);
      std::cout << "Created " << subfolder_abs_path << std::endl;
      serialize_to_pio_impl(subfolder, subfolder_abs_path);
    }
  }
}

void serialize_to_pio(folder const& root, fs::path const& dst)
{
  auto const tmp_path = fs::temp_directory_path() / "pio";
  fs::create_directories(tmp_path);
  fs::create_directories(dst);
  serialize_to_pio_impl(root, tmp_path);
  fs::rename(tmp_path, dst);
  std::cout << "Renamed " << tmp_path << " to " << dst << std::endl;
}

void apply_pio_actions(folder& root)
{
  apply_to_ranges(root,
                  true,
                  {
                      actions::replace_in_range_name("FOLD", "Fold"),
                      actions::replace_in_range_name("__", "%_"),
                      actions::replace_in_range_name("_POT", "%"),
                      actions::replace_in_range_name("Raise1", "2.0bb"),
                      actions::change_color("Fold", 0xff6da2c0),
                      actions::change_color("AllIn", 0xff8b0000),
                      actions::change_color_ends_with("bb", 0xffe9967a),
                      actions::change_color_ends_with("%", 0xffe9967a),
                  });
  apply_to_folders(root, actions::remove_useless_ranges());
  apply_to_folders(root, actions::fix_parent_ranges());
  apply_to_ranges(root,
                  false,
                  {actions::percents_to_bb(),
                   actions::set_unassigned_to_subrange("Fold", 0xff6da2c0),
                   actions::sort_subranges()});
}

void apply_equilab_actions(folder& root)
{
  apply_to_folders(root, actions::remove_useless_ranges());
  // apply_to_ranges(root,
  //                 true,
  //                 {actions::replace_in_range_name("Raise1", "Raise"),
  //                  actions::replace_in_range_name("UTG+4", "HJ"),
  //                  actions::replace_in_range_name("UTG+3", "LJ"),
  //                  actions::change_color_ends_with("bb", 0xffe9967a),
  //                  actions::change_color_ends_with("%", 0xffe9967a),
  //                  actions::change_color("66%", 0xffe9967a),
  //                  actions::change_color("180%", 0xffe9967a)});
  // apply_to_folders(root, actions::replace_in_folder_name("UTG+4", "HJ"));
  // apply_to_folders(root, actions::replace_in_folder_name("UTG+3", "LJ"));
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
  auto const src_path = fs::absolute(fs::canonical(src));
  auto const dst_path = dst;
  // TODO repl
  folder root;
  if (src_format == "pio")
  {
    if (!fs::is_directory(src_path))
    {
      std::cout << "--src must point to a directory when --src-format=pio"
                << std::endl;
      return -1;
    }
    root = pio::parse_folder(src_path);
    apply_pio_actions(root);
  }
  else if (src_format == "equilab")
  {
    if (!fs::is_regular_file(src_path))
    {
      std::cout << "--src must point to a file when --src-format=equilab"
                << std::endl;
      return -1;
    }
    root = equilab::parse(src_path);
    apply_equilab_actions(root);
  }

  if (dst_format == "equilab")
    serialize_to_equilab(root, dst_path);
  else if (dst_format == "pio")
    serialize_to_pio(root, dst_path);
  else if (dst_format == "gtoplus")
    serialize_to_gtoplus(root, dst_path);
}
