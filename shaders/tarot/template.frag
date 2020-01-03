#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
	outColor = color;
}
