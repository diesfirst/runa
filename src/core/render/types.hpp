#ifndef RENDER_TYPES_HPP
#define RENDER_TYPES_HPP

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

struct BufferResources
{
    const vk::Device& device;
    const vk::PhysicalDeviceProperties& physicalDeviceProperties;
    const vk::PhysicalDeviceMemoryProperties& memoryProperties;
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_TYPES_HPP */
