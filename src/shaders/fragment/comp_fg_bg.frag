#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;
layout (constant_id = 2) const int BG = 0;
layout (constant_id = 3) const int FG = 1;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor[];

vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
	vec2 st = gl_FragCoord.xy / resolution;
    vec4 bg = texture(samplerColor[BG], st); 
    vec4 fg = texture(samplerColor[FG], st); 
    outColor = fg + bg * (1.0 - fg.a);
}
