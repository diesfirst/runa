#ifndef RENDER_UBOTYPES_HPP_
#define RENDER_UBOTYPES_HPP_

#include <glm/glm.hpp>

namespace sword
{

namespace render
{

struct FragmentInput
{
    float time;
    float mouseX{0};
    float mouseY{0};	
    float r{1.};
    float g{1.};
    float b{1.};
    float a{1.};
    float brushSize{1.};
    glm::mat4 xform{1.};
};

}; // namespace render

}; // namespace sword

#endif /* end of include guard: UBOTYPES_HPP_ */
