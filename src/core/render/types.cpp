#include "types.hpp"
#include "resource.hpp"

namespace sword
{

namespace render
{

const vk::Buffer* DrawParms::getVertexBuffer() const
{
    if (vertexBufferBlock)
        return &vertexBufferBlock->buffer->getHandle();
    return nullptr;
}

uint32_t DrawParms::getOffset() const
{
    return vertexBufferBlock->offset;
}

} // namespace render

} // namespace sword

