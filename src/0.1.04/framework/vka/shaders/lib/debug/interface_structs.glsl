#ifndef DEBUG_INTERFACE_STRUCTS
#define DEBUG_INTERFACE_STRUCTS



// Intergration domain
#define RAY_DOMAIN_ALL 0
#define RAY_DOMAIN_INDIRECT 1
#define RAY_DOMAIN_AREA_LIGHT 2
#define RAY_DOMAIN_ENV_MAP 3

struct GLSLIndirectBounce
{
	vec3 bouncePos;
	uint pathVertexType;

	vec3 bounceDir;
	uint padding1;

	vec3 localWeight;
	uint padding2;

	vec3 weight;
	uint padding3;
};


struct GLSLDirectRay
{
// Set in ptstate_NextDomain:
	vec3 origin;
	uint pathVertexType;

	vec3 weight;
	uint domain;

// Initialized to default values, must be set manualy
	vec3 bsdf;
	float cosineTerm; // cos(theta)

	vec3 L_i;
	uint padding0;

// Computed in nextDomain/finalizeSample:
	vec3 L_o; // bsdf * (cosineTerm or 1.0) * L_i
	uint padding2;

	vec3 sampleContribution; // externalWeight * L_o
	uint padding3;

// Optional:
	float transmittance;
	float sourceDistance; // optional
	uint padding4[2];
};


struct GLSLPtState
{
	vec3 primaryOrigin;
	uint bounce;

	vec3 primaryDirection;
	uint padding1;

	vec3 weight;
	uint directRayCount;

	vec3 totalContribution;
	uint padding2;
};

#endif