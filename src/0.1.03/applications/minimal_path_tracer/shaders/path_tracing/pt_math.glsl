
#ifndef MATH_H
#define MATH_H

#define PI		3.141592
#define INV_PI  0.318310


#define MAX_FLOAT intBitsToFloat(2139095039)

mat4x3 tangentFrameFromPosNormal(vec3 pos, vec3 normal)
{
	vec3 tangent = (cross(normal, vec3(1.0, 0.0, 0.0)));
	if (length(tangent) < 0.1)
	{
		tangent = (cross(normal, vec3(0.0, 1.0, 0.0)));
	}
	tangent = normalize(tangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	return mat4x3(tangent, bitangent, normal, pos);
}

#endif