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
    int null;
    PaintSample samples[48];
} samples;

#define WIDTH 1600
#define HEIGHT 1600

void main()
{
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    for (int i = 0; i < samples.count; i++)
    {
        vec4 thisColor = vec4(0.);
        float x = samples.samples[i].x;
        float y = samples.samples[i].y;
        vec2 mCoords = vec2(x, y);
        thisColor += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .0025, 0.005);
        thisColor *= vec4(1, 0, 0, 1);
        color += thisColor;
    }
    vec2 mCoords = vec2(ubo.brushX, ubo.brushY);
    vec4 thisColor = vec4(0.);
    thisColor += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .0025, 0.005);
    thisColor *= vec4(0, 1, 0, 1);
    color += thisColor;
//    color *= 0.05;
//    color.rgb *= color.a;
	outColor = color;
}
