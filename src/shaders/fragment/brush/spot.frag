#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

void main()
{
	vec4 color = vec4(0.);
	vec2 st = getCoords();
    vec2 mCoords = getMouseCoords();
    color += fill_aa(circleNormSDF(st - mCoords), ubo.brushSize * .01);
    color.rgb *= vec3(ubo.r, ubo.g, ubo.b);
    color *= ubo.a; //premultiplyinggg
	outColor = color;
}
