#version 460

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 2) uniform uboBuf 
{ 
	float frame;
	float mouseX;
	float mouseY;
    int blur;
} ubo;

layout (binding = 1) uniform sampler2D samplerColor;

vec2 resolution = vec2(WIDTH, HEIGHT);

void main()
{

    int blur_size = ubo.blur; //must always be odd
    if (blur_size < 3)
    {
        outColor = texture(samplerColor, gl_FragCoord.xy / resolution);
        return;
    }
    if (blur_size % 2 == 0)
        blur_size += 1;
    int lower_bound = blur_size / -2;
    int upper_bound = blur_size / 2 + 1;
    float scale = blur_size * blur_size;

	vec2 center = gl_FragCoord.xy;
	vec2 curSt = vec2(0.);
	vec4 color = vec4(0.);

	for (int i = lower_bound; i < upper_bound; i++)
	{
		for (int j = lower_bound; j < upper_bound; j++)
		{
			vec2 shift = vec2(i,j);
			curSt = (center + shift) / resolution; 
			color += texture(samplerColor, curSt) / scale;
		}
	}

	outColor = color;
}
