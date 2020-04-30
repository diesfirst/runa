#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();
	color += stroke(circleSDF(st), .5, .05);
	outColor = vec4(color, 1.);
}
