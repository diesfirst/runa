#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
	vec2 mCoords = getMouseCoords();

	vec2 phase = vec2(mCoords.x, mCoords.y);
	float moving = circleSDF(st + .5 - phase);
	float center = circleSDF(st);

	color += flip(stroke(moving, .5, .05), fill(center, .525));
//	color += vec3(st, .0);

	outColor = color;
}


