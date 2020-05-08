//layout (constant_id = 0) const float WIDTH = 800;
//layout (constant_id = 1) const float HEIGHT = 800;
const float WIDTH = 800;
const float HEIGHT = 800;

layout(location = 0) out vec4 outColor;

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
} ubo;

float PI = 3.14;
float MIN = min(WIDTH, HEIGHT);
vec2 resolution = vec2(MIN, MIN);

vec2 normalizeCoords(vec2 coords)
{
	if (MIN == WIDTH)
	{
		coords.y -= abs(WIDTH - HEIGHT) / 2;
		coords.y = MIN - coords.y;
		coords /= resolution;
		return coords;
	}
	else
	{
		coords.x -= abs(WIDTH - HEIGHT) / 2;
		coords.x = MIN - coords.x;
		coords /= resolution;
		return coords;
	}
}

vec2 getCoords()
{
	return normalizeCoords(gl_FragCoord.xy);
}

vec2 getMouseCoords()
{
	return normalizeCoords(vec2(ubo.mouseX, ubo.mouseY));
}

