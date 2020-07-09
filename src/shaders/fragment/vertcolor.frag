#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 vertColor;

void main()
{
    outColor = vec4(vertColor.x, vertColor.y, vertColor.z, 1.);
    //outColor = vec4(1, 0, 0, 1.);
}
