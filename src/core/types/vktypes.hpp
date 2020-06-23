#ifndef TYPES_VKTYPES_HPP
#define TYPES_VKTYPES_HPP

#define VK_USE_PLATFORM_XCB_KHR

#include <vulkan/vulkan.hpp>

namespace sword
{

namespace render
{

namespace standard 
{

constexpr vk::Format imageFormat = vk::Format::eR8G8B8A8Unorm;

};

};

};

#endif /* end of include guard: TYPES_VKTYPES_HPP */
