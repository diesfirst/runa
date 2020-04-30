#version 460

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout(location = 0) out vec4 outColor;

float PI = 3.14;
	
vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
	vec3 color = vec3(0.);
	vec2 st = gl_FragCoord.xy/resolution;
	color += vec3(0.3, st.y, st.x);
	outColor = vec4(color, 1.);
}
