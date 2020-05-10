float circleSDF(vec2 st)
{
	return length(st - .5) * 2.;
}

float circleNormSDF(vec2 st)
{
    return length(st);
}

float rectSDF(vec2 st, vec2 s)
{
	st = st * 2. - 1.;
	return max(abs(st.x / s.x), abs(st.y / s.y));
}

float crossSDF(vec2 st, float s)
{
	vec2 size = vec2(.25, s);
	return min(rectSDF(st, size.xy), rectSDF(st, size.yx));
}
