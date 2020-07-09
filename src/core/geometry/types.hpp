#ifndef GEOMETRY_TYPES_HPP
#define GEOMETRY_TYPES_HPP

#include <type_traits>
#include <types/vktypes.hpp>
#include <glm/glm.hpp>
#include <cstddef>

namespace sword
{

namespace render {class BufferBlock; }

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
    VertexInfo(uint32_t vertexSize, const uint32_t(&attributeOffsets)[N]) : attributeCount{N},
        bindingDescription{
            .binding = 0,
            .stride = vertexSize,
            .inputRate = vk::VertexInputRate::eVertex}
    {
        static_assert(N < MAX_ATTRIBUTE_COUNT);
        for (int i = 0; i < N; i++)
        {
            attrbuteDescriptions[i] = vk::VertexInputAttributeDescription{
                .location = i,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = attributeOffsets[i]};
        }
    }
    constexpr size_t getAttributeCount() const { return attributeCount; }
    constexpr const vk::VertexInputBindingDescription* getPBindingDescription() const { return &bindingDescription; }
    constexpr const vk::VertexInputAttributeDescription* getPAttributeDescriptions() const { return attrbuteDescriptions.data(); }
private:
    vk::VertexInputBindingDescription bindingDescription;
    size_t attributeCount{0};
    std::array<vk::VertexInputAttributeDescription, MAX_ATTRIBUTE_COUNT> attrbuteDescriptions;
};


}; // namespace geo

}; // namespace sword

#endif /* end of include guard: GEOMETRY_TYPES_HPP */
