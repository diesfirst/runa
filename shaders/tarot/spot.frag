#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
    vec2 mCoords = getMouseCoords();
    color += fill(circleNormSDF(st - mCoords), .01);
    color *= vec4(ubo.r, ubo.g, ubo.b, 1.0);
	outColor = color;
}
