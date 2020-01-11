#version 460
#include "constants.glsl"
#include "stroke.glsl"


void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();
	color += stroke(st.x, .5, .15);
	outColor = vec4(color, 1.);
}
