#version 460

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor;


vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
	vec2 st = gl_FragCoord.xy / resolution;

	outColor = texture(samplerColor, st+.1);
}
