#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();

	float x = (ubo.frame / 100) * .15 - .15;
	vec2 phase = vec2(x, .0);
	float left = circleSDF(st + phase);
	float right = circleSDF(st - phase);

	color += flip(stroke(left, .5, .05), fill(right, .525));

	outColor = vec4(color, 1.);
}
