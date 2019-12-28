#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();
	color.b += circleSDF(st) / 3;
	float n_frame = fract(ubo.frame / 5.);
	color.r += stroke(circleSDF(st), .5, .05) * n_frame;
	outColor = vec4(color, 1.);
}
