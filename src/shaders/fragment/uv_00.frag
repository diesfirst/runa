#version 460

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout(location = 0) out vec4 outColor;

void main()
{
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
	outColor = vec4(st.r, st.g, 0.0, 1.);
}
