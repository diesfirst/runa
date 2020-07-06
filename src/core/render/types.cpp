#include "types.hpp"
#include "resource.hpp"

namespace sword
{

namespace render
{

const vk::Buffer* DrawParms::getVertexBuffer() const
{
    return &vertexBufferBlock->buffer->getHandle();
}

} // namespace render

} // namespace sword

