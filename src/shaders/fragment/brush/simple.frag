#version 460

#include "stroke.glsl"
#include "sdf.glsl"
#include "ubo.glsl"

layout(location = 0) out vec4 outColor;

#define WIDTH 800
#define HEIGHT 800

void main()
{
	vec4 color = vec4(0.);
	vec2 st = gl_FragCoord.xy / vec2(WIDTH, HEIGHT);
    vec2 mCoords = vec2(ubo.brushX, ubo.brushY);
    color += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .0025, 0.01);
    color.rgb *= vec3(.2, .7, .7);
    color *= ubo.a; //premultiplyin
	outColor = color;
}
