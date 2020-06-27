#version 460

layout(location = 0) out vec4 outColor;

void main()
{
    vec2 st = gl_FragCoord.xy / vec2(2048, 2048);
    outColor = vec4(st.x, st.y, 0, 1);
}
