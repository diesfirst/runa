#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;
layout (constant_id = 2) const int START_INDEX = 0;
layout (constant_id = 3) const int END_INDEX = 1;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor[];

layout(set = 0, binding = 0) uniform uboBuf 
{ 
    float time;
	float mouseX;
	float mouseY;
    float r;
    float g;
    float b;
    float a;
    float brushSize;
    mat4 xform;
} ubo;


vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{
    vec4 st = vec4(gl_FragCoord.xy / resolution, 1., 1.);
    st = ubo.xform * st;
	vec4 color = vec4(0.);

    for (int i = START_INDEX; i <= END_INDEX; i++)
    {
        vec4 newColor = texture(samplerColor[i], st.xy); 
        float complement = 1.0 - newColor.a;
        color = newColor + color * complement;
    }

	outColor = color;
}
