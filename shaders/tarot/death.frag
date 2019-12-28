#version 460
#include "constants.glsl"

void main()
{
	vec3 color = vec3(0.);
	vec2 st = getCoords();

	color += step(.5, (st.x + st.y) * .5);

	outColor = vec4(color, 1.);
}
