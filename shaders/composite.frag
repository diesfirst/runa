#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform uboBuf 
{ 
	float frame;
	float mouseX;
	float mouseY;
    int blur;
    float r;
    float g;
    float b;
    int layerId;
} ubo;

layout (binding = 1) uniform sampler2D samplerColor[];

vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
    vec2 st = gl_FragCoord.xy / resolution;
	vec4 color = vec4(0.);

    for (int i = 0; i < 4; i++)
    {
        vec4 newColor = texture(samplerColor[i], st); 
        float complement = 1.0 - newColor.a;
        color.rgb = newColor.rgb * newColor.a + color.rgb * color.a * complement;
        color.a = newColor.a + color.a * complement;
    }

	outColor = color;
}
