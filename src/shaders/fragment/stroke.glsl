float stroke(float x, float s, float w)
{
	float d = step(s, x + w * .5) - step(s, x - w * .5);
	return clamp(d, 0., 1.);
}

float fill(float x, float size)
{
	return 1. - step(size, x);
}

float flip(float v, float pct)
{
	return mix(v, 1. - v, pct);
}

float fill_aa(float x, float size, float delta)
{
    return 1. - smoothstep(size - delta, size + delta, x);
}

float stroke_aa(float x, float s, float w, float delta)
{
    float d = smoothstep(s - delta, s, x + w * .5) - smoothstep(s - delta, s, x - w * .5);
    return clamp(d, 0., 1.);
}
