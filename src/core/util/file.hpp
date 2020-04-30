#ifndef UTIL_FILE_HPP
#define UTIL_FILE_HPP

#include <filesystem>
#include <vector>

namespace sword
{

const std::string toSpv(const std::string_view path, bool has_ext = false);
const std::string toRel(const std::filesystem::path p, const std::filesystem::path root, bool inc_ext = false);
const std::vector<std::string> getPaths(std::string_view root, bool inc_ext = false);

}; // namespace sword

#endif /* end of include guard: UTIL_FILE_HPP */
