#version 460

layout (constant_id = 0) const float WIDTH = 800;
layout (constant_id = 1) const float HEIGHT = 800;

const float width = 800;
const float height = 800;

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 st = gl_FragCoord.xy / vec2(width, height);
    st -= vec2(0.5, 0.5);
    float r = sqrt(st.x * st.x + st.y *st.y);
    float R = sin(r * 19);
    float G = cos(r * 5);
    float B = cos(r * 2.5);
    vec3 cd = vec3(R, G, B);
    cd *= 1;
    outColor = vec4(cd.x, cd.y, cd.z, 1.0);
}
