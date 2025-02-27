#pragma once
#include <vka/core/core_utility/types.h>
#include <vka/core/resources/cachable/RenderPass.h>

#include <imgui_impl_vulkan.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace vka
{

#define GVAR_MAX_STRING_LENGHT 2048

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
	GVAR_DISPLAY_FLOAT,
	GVAR_DISPLAY_UNORM,
	GVAR_DISPLAY_UINT,
	GVAR_ENUM,
	GVAR_UINT_RANGE,
	GVAR_FLOAT_RANGE,
	GVAR_TEXT_INPUT,
	GVAR_FILE_INPUT,
	GVAR_DISPLAY_TEXT,
	GVAR_FILE_OUTPUT,
};


struct GVar_Val
{
	bool     v_bool;
	uint32_t v_uint;
	int      v_int;
	float    v_float;
	float    v_vec3[3];
	std::vector<char> v_char_array;
	bool	 v_focus = false;
	GVar_Val() = default;
	GVar_Val(std::string s);
	GVar_Val(bool b);
	GVar_Val(uint32_t i);
	GVar_Val(int i);
	GVar_Val(float f);
	GVar_Val(float x, float y, float z);
	glm::vec4 getVec4();
	glm::vec3 getVec3();
	void setVec3(glm::vec3 vec);
	uint32_t  bool32();

	bool equals(const GVar_Val &other, GVar_Type type) const;
};


struct GVar_Range
{
	GVar_Val min;
	GVar_Val max;
};

struct GVar_Set
{
	GVar_Range               range;
	std::vector<std::string> list;
	GVar_Set();
	GVar_Set(std::vector<std::string> li);
	GVar_Set(GVar_Val l, GVar_Val r);
};


enum GuiFlags
{
	GUI_FLAGS_MENU_BAR = 1 << 0,
	GUI_FLAGS_OPEN_NODES = 1 << 1,
};

enum GVarFlags
{
	GVAR_FLAGS_NO_LOAD = 1 << 0,
	GVAR_FLAGS_V2 = 1 << 1,
};

struct GVar
{
  private:
	inline static std::vector<GVar *> all = std::vector<GVar *>();

  public:
	std::string id;
	GVar_Val    val;
	GVar_Type   type;
	int         sortId;
	GVar_Set    set;
	uint32_t    flags = 0;


	DELETE_COPY_CONSTRUCTORS(GVar)

	bool operator==(const GVar &other) const;
	bool compareValue(const GVar &other) const;
	GVar(std::string p, GVar_Val v, GVar_Type t, int s);
	GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet);
	GVar(std::string p, GVar_Val v, GVar_Type t, int s, uint32_t flags);
	GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet, uint32_t flags);
	GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list);
	GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list, uint32_t flags);

	void writeToJson(json &j);
	void readFromJson(json &j);
	bool addToGui(uint32_t guiFlags);

	static bool                holdsFocus();
	static std::vector<GVar *> filterMask(std::vector<GVar *> gvar, uint32_t mask);
	static std::vector<GVar *> filterSortID(std::vector<GVar *> gvar, uint32_t sortID);
	static bool addToGui(std::vector<GVar *> gvar, std::string category, uint32_t guiFlags = 0);
	static void                store(std::vector<GVar *> gvar, std::string path);
	static void                load(std::vector<GVar *> gvar, std::string path);
	static std::vector<GVar *> getAll();
	static void                loadAll(std::string path);
	static void                storeAll(std::string path);
	template <typename CAT>
	static std::vector<bool> addAllToGui(uint32_t guiFlags = 0)
	{
		std::vector<bool> changed;
		for (CAT cat = eIterator_begin<CAT>()(); cat != eIterator_end<CAT>()(); cat = eIterator_next<CAT>()(cat))
		{
			std::string         name  = eString_val<CAT>()(cat);
			std::vector<GVar *> gvars = GVar::filterSortID(GVar::getAll(), cat);
			changed.push_back(false);
			if (guiFlags & GUI_FLAGS_MENU_BAR)
			{
				if (ImGui::BeginMenuBar())
				{
					changed.back() = GVar::addToGui(gvars, name, guiFlags);
					ImGui::EndMenuBar();
				}
			}
			else
			{
				if (guiFlags & GUI_FLAGS_OPEN_NODES)
				{
					ImGui::SetNextItemOpen(true);
				}
				
				if (ImGui::CollapsingHeader(name.c_str()))
				{
					changed.back() = GVar::addToGui(gvars, name, guiFlags);
				}
			}
		}
		return changed;
	}
};

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