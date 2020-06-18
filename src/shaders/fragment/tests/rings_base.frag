#version 460
#include "constants.glsl"
#include "stroke.glsl"
#include "sdf.glsl"

vec4 ring(vec2 st, vec2 pos, float radius, float width, vec3 rgb)
{
	vec4 color = vec4(.0);
//	vec2 st = getCoords();
//   color += fill_aa(circleNormSDF(st - pos), radius, 0.0025);
	color += stroke_aa(circleNormSDF(st - pos), radius, width, 0.0025);
//    color += circleNormSDF(st - pos);
    color.xyz *= rgb;
	return color;
}

float rand(float seed)
{
    return fract(sin(seed) * 1200.0);
}

vec2 rand2(float seed)
{
    return vec2(rand(seed * 515), rand(seed * -325));
}

vec3 rand3(float seed)
{
    return vec3(rand(seed * 134), rand(seed * -3124), rand(506));
}

vec4 over(vec4 A, vec4 B)
{
    float alpha = A.a + B.a * ( 1.0 - A.a );
    vec3 res = (A.rgb + B.rgb * ( 1.0 - A.a));
    return vec4(res, alpha);
}

#define ring_count 500
#define n_cells 9

const vec2 cell_vecs[n_cells] = {{0, 0}, {0, 1}, {1, 0}, {-1, 0}, {0, -1}, 
    {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

void main()
{
    vec4 color = vec4(.9, 0.6, .5, 1.);
    vec2 st = gl_FragCoord.xy / RES;
    for (int i = 0; i < ring_count; i++)
    {
        vec2 pos = rand2(i * 23);
        for (int j = 0; j < n_cells; j++)
        {
            vec4 ring_color = ring(st, pos + cell_vecs[j], rand(i * 23) * .06, .02, rand3(i * 7));
            color = over(ring_color, color);
        }
    }
    outColor = color;
}
