#include <filesystem>
#include <functional>

#include <prc/folder.hpp>
#include <prc/range.hpp>

namespace prc::actions
{
using range_action = std::function<void(range&, std::filesystem::path const&)>;
using folder_action =
    std::function<bool(folder&, std::filesystem::path const&)>;

inline namespace range_actions
{
range_action replace_in_range_name(std::string const& old_str,
                                   std::string const& new_str);
range_action change_color(std::string const& range_name, int rgb);
range_action change_color_ends_with(std::string const& str, int rgb);
range_action move_subrange_at_end(std::string const& range_name);
range_action set_unassigned_to_subrange(std::string const& range_name, int rgb);
range_action count_max_ranges();
}

inline namespace folder_actions
{
folder_action fix_parent_ranges();
folder_action nest_parent_ranges();
folder_action remove_empty_ranges();
folder_action replace_in_folder_name(std::string const& old_str,
                                     std::string const& new_str);
}
}
