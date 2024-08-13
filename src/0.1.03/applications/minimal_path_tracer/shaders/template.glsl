
#ifndef FORMAT1
#define FORMAT1 rgba32f
#endif


#define SHADER_BLOCK_3_FRAME_VIEW_PARAMS(OFFSET)							 \
layout(binding = OFFSET) uniform FRAME									 \
{																		 \
	GLSLFrame frame;													 \
};																		 \
layout(binding = OFFSET + 1) uniform VIEW								 \
{																		 \
	GLSLView view;													     \
};																		 \
layout(binding = OFFSET + 2) uniform PARAMS								 \
{																		 \
	GLSLParams params;													 \
};

#define SHADER_BLOCK_2_FRAME_PARAMS										 \
layout(binding = OFFSET) uniform FRAME									 \
{																		 \
	Frame frame;														 \
};																		 \
layout(binding = OFFSET + 1) uniform PARAMS								 \
{																		 \
	GlobalParams params;												 \
};

#define SHADER_BLOCK_10_USCENE(OFFSET)										 \
layout(binding = OFFSET + 0) readonly buffer VERTICES { GLSLVertex vertices[]; }; \
layout(binding = OFFSET + 1) readonly buffer INDICES { uint indices[]; }; \
layout(binding = OFFSET + 2) readonly buffer MODEL_OFFSETS { VKAModelDataOffset modelOffsets[]; }; \
layout(binding = OFFSET + 3) readonly buffer SURFACE_OFFSETS { uint surfaceIndexOffsets[]; }; \
layout(binding = OFFSET + 4) readonly buffer MATERIALS { GLSLMaterial materials[]; }; \
layout(binding = OFFSET + 5) readonly buffer AREA_LIGHTS { VKAAreaLight areaLights[]; }; \
layout(binding = OFFSET + 6) uniform accelerationStructureEXT as; \
layout(binding = OFFSET + 7) uniform sampler smp; \
layout(binding = OFFSET + 8) uniform texture2D tex[]; \
layout(binding = OFFSET + 9) uniform sampler2D envMap;










#define COMPUTE_SHADER_CHECK_ID_FRAME									 \
ivec2 gID = ivec2(gl_GlobalInvocationID.xy);							 \
if(gID.x >= frame.width || gID.y >= frame.height){return;}

#define COMPUTE_SHADER_ID_IN layout(local_size_x_id = 0, local_size_y_id = 1, local_size_z_id = 2) in;