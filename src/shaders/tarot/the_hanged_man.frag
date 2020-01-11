#version 460
#include "constants.glsl"
#include "stroke.glsl"


void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();
	float sdf = .5 + (st.x - st.y) * .5;
	float sdf_inv = (st.x + st.y) * .5;
	color += stroke(sdf, .5, .1);
	color += stroke(sdf_inv, .5, .1);
	outColor = vec4(color, 1.);
}
