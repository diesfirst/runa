#version 460

#include "ubo.glsl"

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor[1];

vec2 resolution = vec2(800, 800);

void main()
{
    vec4 st = vec4(gl_FragCoord.xy / resolution, 1., 1.);
    st = ubo.xform * st;
	vec4 color = vec4(0.);

    for (int i = 0; i < 1; i++)
    {
        vec4 newColor = texture(samplerColor[i], st.xy); 
        float complement = 1.0 - newColor.a;
        color = newColor + color * complement;
    }

	outColor = color;
}
