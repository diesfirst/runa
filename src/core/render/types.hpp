#ifndef RENDER_TYPES_HPP
#define RENDER_TYPES_HPP

#include <types/vktypes.hpp>

namespace sword
{

namespace render
{

struct BufferBlock;

struct BufferResources
{
    const vk::Device& device;
    const vk::PhysicalDeviceProperties& physicalDeviceProperties;
    const vk::PhysicalDeviceMemoryProperties& memoryProperties;
};

struct DrawParms
{
    constexpr DrawParms() = default;
    constexpr DrawParms(BufferBlock* bufferBlock, uint32_t numberOfVerts, uint32_t firstVertex) : 
        vertexBufferBlock{bufferBlock}, vertexCount{numberOfVerts}, firstVertex{firstVertex}
    {}
    const vk::Buffer* getVertexBuffer() const;
    constexpr uint32_t getVertexCount() const { return vertexCount; } 
    constexpr uint32_t getFirstVertex() const { return firstVertex; }
private:
    BufferBlock* vertexBufferBlock{nullptr};
    uint32_t vertexCount{3}; //trianlge is default
    uint32_t firstVertex{0};
};


}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_TYPES_HPP */
