#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec4 color = vec4(0.);
    vec4 userColor = vec4(ubo.r, ubo.g, ubo.b, 1.);
	vec2 st = getCoords();
	vec2 mCoords = getMouseCoords();
	//color.b += circleSDF(st) / 3;
	color += stroke(circleSDF(st + .5 - mCoords), .25, .025) * .1;
    color *= userColor;
	outColor = color;
}
