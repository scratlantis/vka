#pragma once
#include <vka/core/resources/cachable/RenderPass.h>
#include <vka/core/core_utility/types.h>

namespace vka
{
void configureGui_Default();
void enableGui(RenderPassDefinition rpDef, uint32_t subpassIdx);
void enableGui();
void cmdRenderGui(CmdBuffer cmdBuf, Image target, float x, float y, float width, float height);

void cmdRenderGui(CmdBuffer cmdBuf, Image target);

union GVar_Val
{
	bool     v_bool;
	uint32_t v_uint;
	int      v_int;
	float    v_float;
	float    v_vec3[3];

	GVar_Val(bool b)
	{
		v_bool = b;
	}
	GVar_Val(uint32_t i)
	{
		v_uint = i;
	}
	GVar_Val(int i)
	{
		v_int = i;
	}
	GVar_Val(float f)
	{
		v_float = f;
	}
	GVar_Val(float x, float y, float z)
	{
		v_vec3[0] = x;
		v_vec3[1] = y;
		v_vec3[2] = z;
	}
	glm::vec4 getVec4()
	{
		return glm::vec4(v_vec3[0], v_vec3[1], v_vec3[2], 0);
	};
	uint32_t bool32()
	{
		return v_bool ? 1 : 0;
	};
};

typedef std::vector<std::string> GVar_Enum_t;

// Type
enum GVar_Type
{
	GVAR_EVENT,
	GVAR_BOOL,
	GVAR_FLOAT,
	GVAR_UNORM,
	GVAR_INT,
	GVAR_UINT,
	GVAR_VEC3,
	GVAR_DISPLAY_VALUE,
	GVAR_ENUM,
};

struct GVar
{
	std::string path;
	GVar_Val    val;
	GVar_Type   type;
	uint32_t    sortId;
	GVar_Enum_t enumVal;
};

namespace gvar_gui
{
void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories, VkRect2D_OP viewport);
}

namespace shader_console_gui
{
void buildGui(VkRect2D_OP viewport);
}

}        // namespace vka