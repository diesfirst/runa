#ifndef GEOMETRY_TYPES_HPP
#define GEOMETRY_TYPES_HPP

#include <type_traits>
#include <types/vktypes.hpp>
#include <glm/glm.hpp>
#include <cstddef>

namespace sword
{

namespace geo
{

constexpr size_t MAX_ATTRIBUTE_COUNT = 5;

struct AttributeInfo
{
    //later this should be the element of the array we pass to make a VertexInfo
};


struct VertexInfo
{
    VertexInfo() = default;
    template<size_t N>
    VertexInfo(const size_t vertexSize, const uint32_t(&offsets)[N]) : attributeCount{N}
    {
        static_assert(N < MAX_ATTRIBUTE_COUNT);
        bindingDescription.setBinding(0);
        bindingDescription.setStride(vertexSize);
        bindingDescription.setInputRate(vk::VertexInputRate::eVertex);
        for (int i = 0; i < N; i++)
        {
            attrbuteDescriptions[i] = vk::VertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsets[i]};
        }
    }
    vk::VertexInputBindingDescription bindingDescription;
    std::array<vk::VertexInputAttributeDescription, MAX_ATTRIBUTE_COUNT> attrbuteDescriptions;
    size_t attributeCount{0};
};


}; // namespace geo

}; // namespace sword

#endif /* end of include guard: GEOMETRY_TYPES_HPP */
