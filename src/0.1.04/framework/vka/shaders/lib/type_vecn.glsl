#ifndef TYPE_VECN_H
#define TYPE_VECN_H

#ifndef VECN_DIM
#define VECN_DIM 2
#endif


#if VECN_DIM == 2
#define vecN vec2
#define ivecN ivec2
#define uvecN uvec2

#define ALIGNED_SIZE_vecN 8U
#define ALIGNED_SIZE_ivecN 8U
#define ALIGNED_SIZE_uvecN 8U

#elif VECN_DIM == 3

#define ALIGNED_SIZE_vecN 16U
#define ALIGNED_SIZE_ivecN 16U
#define ALIGNED_SIZE_uvecN 16U

#define vecN vec3
#define ivecN ivec3
#define uvecN uvec3
#endif

#endif // TYPE_VECN_H