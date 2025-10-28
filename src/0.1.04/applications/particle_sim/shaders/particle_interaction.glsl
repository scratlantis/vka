


void resolveBorderCollision(inout PARTICLE_TYPE particle, vecN rangeMin, vecN rangeMax, float radius, float damping)
{
	vecN deltaLow = particle.pos - (rangeMin + vecN(radius));
	vecN deltaHigh = rangeMax - vecN(radius) - particle.pos;

	if(deltaLow.x < 0.0)
	{
		particle.pos.x -= deltaLow.x;
		particle.vel.x *= -damping;
	}
	else if(deltaHigh.x < 0.0)
	{
		particle.pos.x += deltaHigh.x;
		particle.vel.x *= -damping;
	}
	if(deltaLow.y < 0.0)
	{
		particle.pos.y -= deltaLow.y;
		particle.vel.y *= -damping;
	}
	else if(deltaHigh.y < 0.0)
	{
		particle.pos.y += deltaHigh.y;
		particle.vel.y *= -damping;
	}
#if VECN_DIM == 3
	if(deltaLow.z < 0.0)
	{
		particle.pos.z -= deltaLow.z;
		particle.vel.z *= -damping;
	}
	else if(deltaHigh.z < 0.0)
	{
		particle.pos.z += deltaHigh.z;
		particle.vel.z *= -damping;
	}
#endif
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








// Surface interaction:
//void resolve_border_collision(inout GLSLParticle particle, vec2 rangeMin, vec2 rangeMax, float damping)
//{
//	float radius = 0.5 * pc.pointSize;
//	vec2 deltaLow = particle.pos - (rangeMin + vec2(radius));
//	vec2 deltaHigh = rangeMax - vec2(radius) - particle.pos;
//
//	if(deltaLow.x < 0.0)
//	{
//		particle.pos.x -= deltaLow.x;
//		particle.vel.x *= -damping;
//	}
//	else if(deltaHigh.x < 0.0)
//	{
//		particle.pos.x += deltaHigh.x;
//		particle.vel.x *= -damping;
//	}
//	if(deltaLow.y < 0.0)
//	{
//		particle.pos.y -= deltaLow.y;
//		particle.vel.y *= -damping;
//	}
//	else if(deltaHigh.y < 0.0)
//	{
//		particle.pos.y += deltaHigh.y;
//		particle.vel.y *= -damping;
//	}
//}
//
//void mouse_force(vec2 mouseScreenCoord, vec2 rangeMin, vec2 rangeMax, float radius)
//{
//	vec2 localMouseCoord = (mouseScreenCoord-rangeMin)*(rangeMax - rangeMin);
//	vec2 dir = localMouseCoord - elem.pos;
//	float dist = length(dir);
//	dir = normalize(dir);
//	if(dist < radius)
//	{
//		float weight = kernelWeight_smooth(dist, radius) * kernelNormalization_smooth(radius)*0.001;
//		return dir * weight * 100.0;
//	}
//	return vec2(0.0);
//}



// Mouse interaction: