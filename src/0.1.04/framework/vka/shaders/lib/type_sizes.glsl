
#ifndef TYPE_SIZES_H
#define TYPE_SIZES_H
// This header defines the sizes of various data types in bytes.


#define ALIGNED_SIZE(X) ALIGNED_SIZE_##X

#define ALIGNED_SIZE_uint 4U
#define ALIGNED_SIZE_float 4U

#define ALIGNED_SIZE_vec2 8U
#define ALIGNED_SIZE_vec3 16U
#define ALIGNED_SIZE_vec4 16U

#define ALIGNED_SIZE_ivec2 8U
#define ALIGNED_SIZE_ivec3 16U
#define ALIGNED_SIZE_ivec4 16U

#define ALIGNED_SIZE_uvec2 8U
#define ALIGNED_SIZE_uvec3 16U
#define ALIGNED_SIZE_uvec4 16U

#endif // TYPE_SIZES_H