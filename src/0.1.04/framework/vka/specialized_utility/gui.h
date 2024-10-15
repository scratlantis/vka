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

void setGuiDimensions(Rect2D<float> rect);

#define GVAR_MAX_STRING_LENGHT 256

struct GVar_Val
{
	bool     v_bool;
	uint32_t v_uint;
	int      v_int;
	float    v_float;
	float    v_vec3[3];

	std::vector<char> v_char_array;

	/*GVar_Val() = default;
	~GVar_Val() = default;*/
	
	GVar_Val(std::string s)
	{
	    v_char_array = std::vector<char>(s.begin(), s.end());
		v_char_array.push_back('\0');
		VKA_ASSERT(v_char_array.size() < GVAR_MAX_STRING_LENGHT);
		v_char_array.resize(GVAR_MAX_STRING_LENGHT);
	}

	GVar_Val(bool b)
	{
		v_bool = b;
	}
	GVar_Val(uint32_t i)
	{
		v_uint = i;
		v_int = static_cast<int>(i);
	}
	GVar_Val(int i)
	{
		v_int = i;
		v_uint = static_cast<uint32_t>(i);
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
	glm::vec3 getVec3()
	{
		return glm::vec3(v_vec3[0], v_vec3[1], v_vec3[2]);
	};
	uint32_t bool32()
	{
		return v_bool ? 1 : 0;
	};
};

struct GVar_Range
{
	GVar_Val min;
	GVar_Val max;
};

//typedef std::vector<std::string> GVar_Enum_t;

struct GVar_Set
{
	GVar_Range range;
	std::vector<std::string> list;
	GVar_Set() :
	    range({0, 0})
	{
	}
	GVar_Set(std::vector<std::string> li) :
	    list(li), range({ 0, 0 })
	{
	}
	GVar_Set(GVar_Val l, GVar_Val r) : range({ l, r })
	{
	}
};

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
	GVAR_VEC3_RANGE,
	GVAR_DISPLAY_VALUE,
	GVAR_ENUM,
	GVAR_UINT_RANGE,
	GVAR_FLOAT_RANGE,
	GVAR_TEXT_INPUT,
};

enum GuiFlags
{
	GUI_FLAGS_NO_LOAD = 1 << 0,
};
struct GVar
{
	std::string id;
	GVar_Val    val;
	GVar_Type   type;
	int			sortId;
	GVar_Set	set;
	uint32_t    flags = 0;

	bool operator==(const GVar &other) const
	{
		return id == other.id;
	}

	bool compareValue(const GVar &other) const
	{
		if (type == GVAR_VEC3)
		{
			return val.v_vec3[0] == other.val.v_vec3[0] && val.v_vec3[1] == other.val.v_vec3[1] && val.v_vec3[2] == other.val.v_vec3[2];
		}
		else if (type == GVAR_FLOAT || type == GVAR_FLOAT_RANGE || type == GVAR_UNORM)
		{
			return val.v_float == other.val.v_float;
		}
		else if (type == GVAR_INT)
		{
			return val.v_int == other.val.v_int;
		}
		else if (type == GVAR_UINT || type == GVAR_ENUM || type == GVAR_UINT_RANGE)
		{
			return val.v_uint == other.val.v_uint;
		}
		else if (type == GVAR_BOOL)
		{
			return val.v_bool == other.val.v_bool;
		}
		else if (type == GVAR_TEXT_INPUT)
		{
			return val.v_char_array == other.val.v_char_array;
		}
		else if (type == GVAR_VEC3_RANGE)
		{
			return val.v_vec3[0] == other.val.v_vec3[0] && val.v_vec3[1] == other.val.v_vec3[1] && val.v_vec3[2] == other.val.v_vec3[2];
		}
		else
		{
			return true;
		}
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s)
	{
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s),
	    set(aSet)
	{
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s, uint32_t flags) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s),
		flags(flags)
	{
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet, uint32_t flags) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s),
	    set(aSet),
		flags(flags)
	{
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s),
	    set( {list} )
	{
	}

	GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list, uint32_t flags) :
	    id(p),
	    val(v),
	    type(t),
	    sortId(s),
	    set({list}),
	    flags(flags)
	{
	}

};



void storeGVar(std::vector<GVar *> gvar, std::string path);
void loadGVar(std::vector<GVar *> gvar, std::string path);

namespace gvar_gui
{
void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories, VkRect2D_OP viewport);
}

namespace gvar_gui_v2
{
void addGVar(GVar *gv);
void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories, VkRect2D_OP viewport);
void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories);
}

namespace shader_console_gui
{
void buildGui(VkRect2D_OP viewport);
}

}        // namespace vka

namespace std
{
template <>
struct hash<vka::GVar>
{
	std::size_t operator()(const vka::GVar &gvar) const
	{
		return std::hash<std::string>()(gvar.id);
	}
};
}        // namespace std