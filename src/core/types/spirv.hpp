#ifndef TYPES_SPIRV_HPP
#define TYPES_SPIRV_HPP

#include <vector>
#include <cstdint>
#include <shaderc/shaderc.hpp>

namespace sword
{

class Spirv
{
public:
private:
    std::vector<uint32_t> code;
};

}; // namespace sword

#endif /* end of include guard: TYPES_SPIRV_HPP */
