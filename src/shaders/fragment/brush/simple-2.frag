#version 460

#include "stroke.glsl"
#include "sdf.glsl"
#include "ubo.glsl"

layout(location = 0) out vec4 outColor;

struct PaintSample
{
    float x;
    float y;
};

layout(set = 0, binding = 2) uniform sampleArray
{
    int count;
    PaintSample samples[];
} samples;

#define WIDTH 1600
#define HEIGHT 1600

void main()
{
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    vec2 mCoords = vec2(ubo.brushX, ubo.brushY);
    color += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .0025, 0.005);
    color.rgb *= vec3(.9, .861, .5);
//    color *= 0.05;
//    color.rgb *= color.a;
	outColor = color;
}
