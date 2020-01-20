#version 460

#include "stroke.glsl"
#include "sdf.glsl"

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;
layout (constant_id = 2) const int W_WIDTH = 500;
layout (constant_id = 3) const int W_HEIGHT = 500;

layout(set = 0, binding = 0) uniform uboBuf 
{ 
    float time;
	float mouseX;
	float mouseY;
    float r;
    float g;
    float b;
    float a;
    float brushSize;
    mat4 xform;
} ubo;

layout(location = 0) out vec4 outColor;

void main()
{
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    vec2 mCoords = vec2(ubo.mouseX, ubo.mouseY);
    color += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .01);
    color.rgb *= vec3(ubo.r, ubo.g, ubo.b);
    color *= ubo.a; //premultiplying
	outColor = color;
}
