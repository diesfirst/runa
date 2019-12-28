layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform uboBuf 
{ 
	float frame;
} ubo;

float PI = 3.14;
vec2 resolution = vec2(WIDTH, WIDTH);

vec2 getCoords()
{
	vec2 fragCoords = gl_FragCoord.xy;
	fragCoords.y -= abs(WIDTH - HEIGHT) / 2;
	vec2 st = fragCoords / resolution;
	st.y = 1. - st.y;
	return st;
}
