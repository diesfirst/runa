#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec3 color = vec3(.0);
	vec2 st = getCoords();

	float rect = rectSDF(st, vec2(.5, 1.));
	float diag = (st.x + st.y) * .5 + ubo.frame / 100. - .5;
	color += flip(
			fill(rect, .6), 
			stroke(diag, .5, .01));

	outColor = vec4(color, 1.);
}
