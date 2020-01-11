#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

#define N 4
const vec2 offsets[N] = {vec2(.1, 0.), vec2(0., .1), vec2(-.1, 0.), vec2(0., -.1)};
const vec3 colors[N] = {vec3(.9, .4, .2), vec3(.7, .4, .1), vec3(.5, .2, .05), vec3(.3, .1, .025)};

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
    vec2 mCoords = getMouseCoords();
    float angle = ubo.time * 30;
    for (int i = 0; i < N; i++)
    {
        mat2 rot = mat2(
                cos(angle), -1 * sin(angle),
                sin(angle), cos(angle));
        vec2 pos = rot * offsets[i];
        pos *= .5; //scale
        color += fill(circleNormSDF(st - mCoords - pos), .01);
        color *= vec4(colors[i], 1.0);
    }
    color *= vec4(ubo.r, ubo.g, ubo.b, 1.0);
	outColor = color;
}
