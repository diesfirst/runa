#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();

	vec2 phase = vec2(.15, .0);
	float left = circleSDF(st + phase);
	float right = circleSDF(st - phase);

	color += flip(stroke(left, .5, .05), fill(right, .525));

	outColor = vec4(color, 1.);
}
