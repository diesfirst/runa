#include "file.hpp"

namespace sword
{

const std::string toSpv(const std::string_view path, bool has_ext)
{
    auto pos = path.find_last_of('/');
    std::string_view name;
    if (pos == std::string_view::npos)
        name = path;
    else
        name = path.substr(pos + 1);
    std::string spv; 
    if (has_ext)
    {
        pos = path.find_last_of('.');
        name = name.substr(0, pos);
    }
    spv = std::string(name) + ".spv";
    return spv;
}

const std::string toRel(const std::filesystem::path p, const std::filesystem::path root, bool inc_ext)
{
    std::string result;
    std::filesystem::path name;
    auto rel = std::filesystem::relative(p, root);
    if (inc_ext)
        name = rel.filename();
    else
        name = rel.stem();
    if (rel.has_parent_path()) 
        result = rel.parent_path().string() + '/' + name.string();
    else 
        result = rel.stem().string();
    return result;
}

const std::vector<std::string> getPaths(std::string_view root, bool inc_ext)
{
    std::vector<std::string> words;
    auto iter = std::filesystem::recursive_directory_iterator(root);
    auto r = std::filesystem::path(root);
    for (const auto& path : iter) 
    {
        if (path.is_directory()) continue;
        words.push_back(toRel(path, r, inc_ext));
    }
    return words;
}

}; // namespace sword
