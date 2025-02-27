
#ifndef RANDOM_H
#define RANDOM_H

#extension GL_EXT_control_flow_attributes : enable

//https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
/*
    static.frag
    by Spatial
    05 July 2013
*/



// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}



// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }



// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( uint x ) { return floatConstruct(hash((x))); }
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

vec3 random3D( vec3  v ) {
    return vec3(
        floatConstruct(hash(floatBitsToUint(v+vec3(32.2345,9382.918,851.38)))),
        floatConstruct(hash(floatBitsToUint(v+vec3(304.23,304.8310,21.13901)))),
        floatConstruct(hash(floatBitsToUint(v+vec3(491.2302,391.0191,39.39102))))
    );
}

uint hash3f(vec3 v)
{
	return hash(floatBitsToUint(v));
}

float unormNext(inout uint seed)
{
    seed = hash(seed);
	return random(seed);
}

vec3 random3D(uint seed)
{
	return vec3(unormNext(seed),unormNext(seed),unormNext(seed));
}

vec2 random2D(uint seed)
{
	return vec2(unormNext(seed),unormNext(seed));
}

void applyJitter(float coefPos, float coefAngle, inout vec3 pos, inout vec3 dir)
{
	pos += (random3D(pos)-vec3(0.5)) * coefPos*1.0;
	dir += (random3D(dir)-vec3(0.5)) * coefAngle;
	dir = normalize(dir);
}


// Based on Ronja's shader tutorials
// https://www.ronja-tutorials.com/post/026-perlin-noise/

#define EASE_IN(A) (A)*(A)
#define EASE_OUT(A) (1-(EASE_IN(1-(A))))
#define EASE_IN_OUT(A) mix(EASE_IN(A),EASE_OUT(A),(A))
#define EASE_IN_OUT3(A) vec3(EASE_IN_OUT((A).x), EASE_IN_OUT((A).y), EASE_IN_OUT((A).z));



float perlinNoise(vec3 val)
{
	vec3 frac = fract(val);
	vec3 a = EASE_IN_OUT3(frac);
	float cellNoiseZ[2];
	[[unroll]]
	for(uint z = 0; z<=1; z++)
	{
		float cellNoiseY[2];
		[[unroll]]
		for(uint y = 0; y<=1; y++)
		{
			float cellNoiseX[2];
			[[unroll]]
			for(uint x = 0; x<=1; x++)
			{
				vec3 cell = floor(val) + vec3(x, y, z);
				vec3 cellDir = random3D(cell) * 2 - 1;
				vec3 compVec = frac - vec3(x , y, z);
				cellNoiseX[x] = dot(cellDir, compVec);
			}
			cellNoiseY[y] = mix(cellNoiseX[0], cellNoiseX[1], a.x);
		}
		cellNoiseZ[z] = mix(cellNoiseY[0], cellNoiseY[1], a.y);
	}
	//return a.z;
	return mix(cellNoiseZ[0], cellNoiseZ[1], a.z);
}

// iteratively construct a bit mask with P(mask[i] = 1) = p
#if 1
uint randomBitMask(float p, uint iterations, inout uint seed)
{
	uint upperMask = 0xFFFFFFFFU; // 32 bits all 1
	uint lowerMask = 0x00000000U; // 32 bits all 0
	float upperP = 1.0;
	float lowerP = 0.0;
	uint middleMask;
	for(uint i = 0; i<iterations; i++)
	{
		uint h = hash(seed); // We assume P(h[i] = 1) = 0.5
		seed = h;
		middleMask = h & lowerMask | (~h) & upperMask;
		float middleP = mix(lowerP, upperP, 0.5);
		if(middleP > p)
		{
			upperMask = middleMask;
			upperP = middleP;
		}
		else
		{
			lowerMask = middleMask;
			lowerP = middleP;
		}
	}
	return middleMask;
}
#else
uint randomBitMask(float p, uint iterations, inout uint seed)
{
	uint mask = 0;
	for(uint i = 0; i<32; i++)
	{
		float rng = unormNext(seed);
		if(rng < p)
		{
			mask |= 1 << i;
		}
	}
	return mask;
}
#endif

float estimateBitPropability(uint mask)
{
	return float(bitCount(mask))/32.0;
}

#endif