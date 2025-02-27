
// source:
// https://github.com/bkaradzic/bgfx/blob/master/examples/09-hdr/common.sh

float reinhard2(float _x, float _whiteSqr)
{
	return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}

vec3 reinhard2(vec3 _x, float _whiteSqr)
{
	return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}