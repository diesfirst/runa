#version 460

#include "stroke.glsl"
#include "sdf.glsl"
#include "ubo.glsl"

layout(location = 0) out vec4 outColor;

struct PaintSample
{
    float x;
    float y;
    float fuck;
    float you;
};

layout(set = 0, binding = 2) uniform sampleArray
{
    int count;
    int null0;
    int null1;
    int null2;
    PaintSample samples[48];
} samples;

#define WIDTH 1600
#define HEIGHT 1600

#define interpCount 36

vec4 over(vec4 A, vec4 B)
{
    float alpha = A.a + B.a * ( 1.0 - A.a );
    vec3 res = (A.rgb + B.rgb * ( 1.0 - A.a));
    return vec4(res, alpha);
}

vec4 colorWithInterp(vec2 sampleA, vec2 sampleB)
{ 
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);

    vec2 AtoB = sampleB - sampleA;
    float stepSize = 1.0 / interpCount;

    for (int i = 0; i < interpCount; i++)
    {
        vec2 curSample = sampleA + AtoB * stepSize * i;
        vec4 thisColor = vec4(0.);
        thisColor += fill_aa(circleNormSDF(st - curSample), ubo.brushSize * .0025, 0.005);
        thisColor *= vec4(.8, .2, .5, 1);
        color = over(thisColor, color);
    }
    return color;
}

void main()
{
    vec4 color = vec4(0.);
    for (int i = 0; i < samples.count; i++)
    {
        float x = samples.samples[i].x;
        float y = samples.samples[i].y;
        vec2 thisCoord = vec2(x, y);
        vec2 nextCoord = vec2(x, y);
        if (i < samples.count - 1)
        {
            nextCoord.x = samples.samples[i + 1].x;
            nextCoord.y = samples.samples[i + 1].y;
        }
        else
        {
            //swap the coords around?
        }
        color = over(colorWithInterp(thisCoord, nextCoord), color);
    }
	outColor = color;
}
