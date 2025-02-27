#ifndef PT_SHADER_STATE_H
#define PT_SHADER_STATE_H

#include "../../lib/debug/interface_structs.glsl"
#include "../../lib/local_structs.glsl"

layout(binding = PT_SHADER_STATE_BINDING_OFFSET) buffer INDIRECT_BOUNCES
{
	GLSLIndirectBounce indirect_bounces[];
};

layout(binding = PT_SHADER_STATE_BINDING_OFFSET + 1) buffer DIRECT_RAYS
{
	GLSLDirectRay direct_rays[];
};

layout(binding = PT_SHADER_STATE_BINDING_OFFSET + 2) buffer PT_STATE
{
	GLSLPtState pt_state;
};


void initPts(Ray primaryRay)
{
	pt_state.primaryOrigin = primaryRay.origin;
	pt_state.primaryDirection = primaryRay.direction;
	pt_state.bounce = 0;
	pt_state.directRayCount = 0;
	pt_state.weight = vec3(1.0);
}

void  nextBounce(Ray ray, uint hitType)
{
	if(pt_state.bounce > 0)
	{
		pt_state.weight = indirect_bounces[pt_state.bounce - 1].weight;
	}
	GLSLIndirectBounce b = indirect_bounces[pt_state.bounce];

	b.pathVertexType = hitType;
	b.bouncePos = ray.origin;
	b.bounceDir = ray.direction;
	b.localWeight = ray.weight;
	b.weight = ray.weight * pt_state.weight;

	indirect_bounces[pt_state.bounce] = b;
	pt_state.bounce++;
}


void finalizeSample()
{
	if(pt_state.directRayCount == 0) return;
	GLSLDirectRay r = direct_rays[pt_state.directRayCount - 1];

	r.L_o = r.bsdf * r.cosineTerm * r.L_i;
	r.sampleContribution = r.weight * r.L_o;
}

void nextSample(uint domain)
{
	if( pt_state.bounce == 0) return;
	finalizeSample();

	GLSLDirectRay r = direct_rays[pt_state.directRayCount];

	r.domain = domain;
	r.origin = indirect_bounces[pt_state.bounce - 1].bouncePos;
	r.pathVertexType = indirect_bounces[pt_state.bounce - 1].pathVertexType;
	r.weight = pt_state.weight;

	r.bsdf = vec3(1.0);
	r.cosineTerm = 1.0;
	r.L_i = vec3(0.0);

	r.transmittance = 1.0;
	r.sourceDistance = TMAX;

	direct_rays[pt_state.directRayCount] = r;
	pt_state.directRayCount ++;
}


void resetPts()
{
	pt_state.bounce = 0;
	pt_state.directRayCount = 0;
}


void setBSDF(vec3 bsdf)
{
	direct_rays[pt_state.directRayCount - 1].bsdf = bsdf;
}


void setCosineTerm(float clampedCosine)
{
	direct_rays[pt_state.directRayCount - 1].cosineTerm = clampedCosine;
}


void setIncomingRadiance(vec3 L_i)
{
	direct_rays[pt_state.directRayCount - 1].L_i = L_i;
}

void setTransmittance(float transmittance)
{
	direct_rays[pt_state.directRayCount - 1].transmittance = transmittance;
}

void setSourceDistance(float sourceDistance)
{
	direct_rays[pt_state.directRayCount - 1].sourceDistance = sourceDistance;
}

void setTotalContribution(vec3 totalContribution)
{
	pt_state.totalContribution = totalContribution;
}

#endif