#ifndef RENDER_TYPES_HPP
#define RENDER_TYPES_HPP

#include <types/vktypes.hpp>
#include <util/debug.hpp>

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
    uint32_t getOffset() const;
    constexpr uint32_t getVertexCount() const { return vertexCount; } 
    constexpr uint32_t getFirstVertex() const { return firstVertex; }
    constexpr void reset() 
    {
        vertexBufferBlock = nullptr;
        vertexCount = 3;
        firstVertex = 0;
    }
private:
    BufferBlock* vertexBufferBlock{nullptr};
    uint32_t vertexCount{3}; //trianlge is default
    uint32_t firstVertex{0};
};

struct RenderParms
{
    static constexpr int MAX_UBO_COUNT = 5;

    RenderParms() = default;

    RenderParms(uint32_t bufferId) : bufferId{bufferId}
    {}

    template<size_t N>
    RenderParms(uint32_t bufferId, const int(&uboIndexes)[N] ) : bufferId{bufferId},
        uboCount{N}
    {
        static_assert(N < MAX_UBO_COUNT);
        memcpy(uboIndices.data(), uboIndexes, sizeof(int) * N);
    }

    constexpr uint32_t getBufferId() const { return bufferId; }
    constexpr uint32_t getUboCount() const { return uboCount; }
    const std::array<int, MAX_UBO_COUNT>& getUboIndices() const { return uboIndices; } 
    size_t uboCount{0};
private:
    uint32_t bufferId{0};
    std::array<int, MAX_UBO_COUNT> uboIndices;
};


}; // namespace render

}; // namespace sword

#endif /* end of include guard: RENDER_TYPES_HPP */
