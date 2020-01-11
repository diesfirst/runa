#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();

	float rect = rectSDF(st, vec2(1.));
	float crux = crossSDF(st, 1.);
	color += fill(rect, .5);
	color *= step(.5, fract(crux * 4.));
	color *= step(1., crux);
	color += fill(crux, .5);
	color += stroke(rect, .65, .05);
	color += stroke(rect, .75, .025);

	outColor = vec4(color, 1.);
}
