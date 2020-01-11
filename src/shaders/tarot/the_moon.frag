#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();

	color += fill(circleSDF(st), .65);
	vec2 offset = vec2(.1, .05);
	color -= fill(circleSDF(st - offset), .5);

	outColor = vec4(color, 1.);
}
