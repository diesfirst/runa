#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

const vec2 offsets[4] = {vec2(.05, 0.), vec2(0., .05), vec2(-.05, 0.), vec2(0., -.05)};

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
    vec2 mCoords = getMouseCoords();
    float angle = ubo.time * 30;
    for (int i = 0; i < 3; i++)
    {
        mat2 rot = mat2(
                cos(angle), -1 * sin(angle),
                sin(angle), cos(angle));
        vec2 pos = rot * offsets[i];
        color += fill(circleNormSDF(st - mCoords - pos), .01);
    }
    color *= vec4(ubo.r, ubo.g, ubo.b, 1.0);
	outColor = color;
}
