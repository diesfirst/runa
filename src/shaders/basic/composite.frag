#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;
layout (constant_id = 2) const int arraySize = 2;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor[];

vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
    vec2 st = gl_FragCoord.xy / resolution;
	vec4 color = vec4(0.);

    for (int i = 0; i < arraySize; i++)
    {
        vec4 newColor = texture(samplerColor[i], st); 
        float complement = 1.0 - newColor.a;
        color = newColor + color * complement;
    }

	outColor = color;
}
