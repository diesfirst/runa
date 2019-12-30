#version 460

layout (constant_id = 0) const float WIDTH = 400;
layout (constant_id = 1) const float HEIGHT = 100;

layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform sampler2D samplerColor;

vec2 resolution = vec2(WIDTH, HEIGHT);

const int BLUR_SIZE = 13; //must always be odd

int lower_bound = BLUR_SIZE / -2;
int upper_bound = BLUR_SIZE / 2 + 1;
float scale = BLUR_SIZE * BLUR_SIZE;

void main()
{
	vec2 center = gl_FragCoord.xy;
	vec2 curSt = vec2(0.);
	vec4 color = vec4(0.);

	for (int i = lower_bound; i < upper_bound; i++)
	{
		for (int j = lower_bound; j < upper_bound; j++)
		{
			vec2 shift = vec2(i,j);
			curSt = (center + shift) / resolution; 
			color += texture(samplerColor, curSt).r / scale;
		}
	}

	vec2 st = gl_FragCoord.xy / resolution;
	outColor = color;
}
