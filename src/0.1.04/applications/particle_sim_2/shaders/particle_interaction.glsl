



vecN getBorderForce(PARTICLE_TYPE particle, vecN rangeMin, vecN rangeMax, float radius)
{
	vecN deltaLow = particle.pos - (rangeMin + vecN(radius*1.0));
	vecN deltaHigh = rangeMax - vecN(radius*1.0) - particle.pos;


	vecN forceLow = -min(vecN(0.0), deltaLow);
	vecN forceHigh = min(vecN(0.0), deltaHigh);

	return (forceLow + forceHigh);
}


void resolveBorderCollision(inout PARTICLE_TYPE particle, vecN rangeMin, vecN rangeMax, float radius, float damping, vecN boundaryVel)
{
	vecN deltaLow = particle.pos - (rangeMin + vecN(radius*0.0));
	vecN deltaHigh = rangeMax - vecN(radius*0.0) - particle.pos;

	vecN boundaryVelLow = max(vecN(0.0), boundaryVel);
	vecN boundaryVelHigh = max(vecN(0.0), -boundaryVel);


	vecN rng = vecN(0.0);
	//rng.x = perlinNoise1D(1.0*particle.pos.x);
	rng.y = perlinNoise1D(100.0*particle.pos.x);
	#if VECN_DIM == 3
	//rng.z = perlinNoise1D(100.0*particle.pos.z);
	#endif

	//deltaLow += rng * 0.001;
	//deltaHigh += rng * 0.001;

	PARTICLE_TYPE copy = particle;

	if(deltaLow.x < 0.0)
	{
		particle.pos.x -= deltaLow.x;
		particle.vel.x *= -damping;
		particle.vel.x += boundaryVelLow.x;
	}
	else if(deltaHigh.x < 0.0)
	{
		particle.pos.x += deltaHigh.x;
		particle.vel.x *= -damping;
		particle.vel.x -= boundaryVelHigh.x;
	}
	if(deltaLow.y < 0.0)
	{
		particle.pos.y -= deltaLow.y;
		particle.vel.y *= -damping;
		particle.vel.y += boundaryVelLow.y;
	}
	else if(deltaHigh.y < 0.0)
	{
		particle.pos.y += deltaHigh.y;
		particle.vel.y *= -damping;
		particle.vel.y -= boundaryVelHigh.y;
	}
#if VECN_DIM == 3
	if(deltaLow.z < 0.0)
	{
		particle.pos.z -= deltaLow.z;
		particle.vel.z *= -damping;
		particle.vel.z += boundaryVelLow.z;
	}
	else if(deltaHigh.z < 0.0)
	{
		particle.pos.z += deltaHigh.z;
		particle.vel.z *= -damping;
		particle.vel.z -= boundaryVelHigh.z;
	}
#endif

	//vecN radN = vecN(radius);
	//vecN delta = min(deltaLow, deltaHigh);
	//delta = clamp(delta/radN, vecN(0.0), vecN(1.0));
	//particle.vel = mix(particle.vel, copy.vel, delta);
	//particle.pos = mix(particle.pos, copy.pos, delta);

}

bool isInBox(PARTICLE_TYPE particle, vecN rangeMin, vecN rangeMax, float radius)
{
	vecN deltaLow = particle.pos - (rangeMin + vecN(radius*0.0));
	vecN deltaHigh = rangeMax - vecN(radius*0.0) - particle.pos;

	if(deltaLow.x < 0.0 || deltaHigh.x < 0.0)
		return false;

	if(deltaLow.y < 0.0 || deltaHigh.y < 0.0)
		return false;

#if VECN_DIM == 3
	if(deltaLow.z < 0.0 || deltaHigh.z < 0.0)
		return false;
#endif

	return true;
}

vecN getCursorForce(vecN cursorCoord, vecN pos, float radius)
{
	vecN dir = cursorCoord - pos;
	float dist = length(dir);
	dir = normalize(dir);
	if(dist < radius)
	{
		float weight = kernelWeight_smooth(dist, radius) * kernelNormalization_smooth(radius)*0.001;
		return dir * weight * 100.0;
	}
	return vecN(0.0);
}





// Mouse interaction: