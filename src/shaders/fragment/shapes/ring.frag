#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

vec4 ring()
{
	vec4 color = vec4(.0);
	vec2 st = getCoords();
	color += stroke(circleSDF(st), .5, .05);
	return color
}
