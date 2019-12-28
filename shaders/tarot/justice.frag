#version 450
#include "constants.glsl"

void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();
	color += step(0.5, st.x);
	outColor = vec4(color, 1.);
}
