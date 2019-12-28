#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();

	float sdf = rectSDF(st, vec2(1.));
	color += stroke(sdf, .5, .125);
	color += fill(sdf, .1);

	outColor = vec4(color, 1.);
}
