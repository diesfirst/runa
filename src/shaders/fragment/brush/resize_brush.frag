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
    vec2 mCoords = vec2(ubo.mouseX, ubo.mouseY);
    color += stroke_aa(circleNormSDF(st - mCoords), ubo.brushSize * .1, 0.05, 0.01);
    color.rgb *= vec3(.1, .5, .9);
    color *= ubo.a; //premultiplyin
	outColor = color;
}
