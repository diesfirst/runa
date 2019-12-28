#version 460
#include "constants.glsl"
#include "stroke.glsl"


void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();
	float offset = cos(st.y * PI) * .15;
	color += stroke(st.x, .28 + offset, .1);
	color += stroke(st.x, .5 + offset, .1);
	color += stroke(st.x, .72 + offset, .1);
	outColor = vec4(color, 1.);
}
