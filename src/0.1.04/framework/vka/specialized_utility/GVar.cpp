#include "GVar.h"
#include <ImFileDialog.h>



namespace vka
{

	// Value
GVar_Val::GVar_Val(std::string s)
{
	v_char_array = std::vector<char>(s.begin(), s.end());
	v_char_array.push_back('\0');
	VKA_ASSERT(v_char_array.size() < GVAR_MAX_STRING_LENGHT);
	v_char_array.resize(GVAR_MAX_STRING_LENGHT);
	v_bool = false;
}

GVar_Val::GVar_Val(bool b)
{
	v_bool = b;
}
GVar_Val::GVar_Val(uint32_t i)
{
	v_uint = i;
	v_int  = static_cast<int>(i);
}
GVar_Val::GVar_Val(int i)
{
	v_int  = i;
	v_uint = static_cast<uint32_t>(i);
}
GVar_Val::GVar_Val(float f)
{
	v_float = f;
}
GVar_Val::GVar_Val(float x, float y, float z)
{
	v_vec3[0] = x;
	v_vec3[1] = y;
	v_vec3[2] = z;
}
glm::vec4 GVar_Val::getVec4()
{
	return glm::vec4(v_vec3[0], v_vec3[1], v_vec3[2], 0);
};
glm::vec3 GVar_Val::getVec3()
{
	return glm::vec3(v_vec3[0], v_vec3[1], v_vec3[2]);
}
void GVar_Val::setVec3(glm::vec3 vec)
{
	v_vec3[0] = vec.x;
	v_vec3[1] = vec.y;
	v_vec3[2] = vec.z;
};
uint32_t GVar_Val::bool32()
{
	return v_bool ? 1 : 0;
};

bool GVar_Val::equals(const GVar_Val &other, GVar_Type type) const
{
	if (type == GVAR_VEC3)
	{
		return v_vec3[0] == other.v_vec3[0] && v_vec3[1] == other.v_vec3[1] && v_vec3[2] == other.v_vec3[2];
	}
	else if (type == GVAR_FLOAT || type == GVAR_FLOAT_RANGE || type == GVAR_UNORM)
	{
		return v_float == other.v_float;
	}
	else if (type == GVAR_INT)
	{
		return v_int == other.v_int;
	}
	else if (type == GVAR_UINT || type == GVAR_ENUM || type == GVAR_UINT_RANGE)
	{
		return v_uint == other.v_uint;
	}
	else if (type == GVAR_BOOL)
	{
		return v_bool == other.v_bool;
	}
	else if (type == GVAR_TEXT_INPUT)
	{
		return v_char_array == other.v_char_array;
	}
	else if (type == GVAR_VEC3_RANGE)
	{
		return v_vec3[0] == other.v_vec3[0] && v_vec3[1] == other.v_vec3[1] && v_vec3[2] == other.v_vec3[2];
	}
	else if (type == GVAR_DISPLAY_UNORM)
	{
		return v_float == other.v_float;
	}
	else if (type == GVAR_FILE_INPUT)
	{
		return std::string(v_char_array.data()) == std::string(other.v_char_array.data());
	}
	else if (type == GVAR_FILE_OUTPUT)
	{
		return std::string(v_char_array.data()) == std::string(other.v_char_array.data());
	}
	else if (type == GVAR_DISPLAY_TEXT)
	{
		return std::string(v_char_array.data()) == std::string(other.v_char_array.data());
	}
	else
	{
		return true;
	}
}

// Value Range
GVar_Set::GVar_Set() :
    range({0, 0})
{
}
GVar_Set::GVar_Set(std::vector<std::string> li) :
    list(li), range({0, 0})
{
}
GVar_Set::GVar_Set(GVar_Val l, GVar_Val r) :
    range({l, r})
{
}


// GVar
bool GVar::operator==(const GVar &other) const
{
	return id == other.id;
}

bool GVar::compareValue(const GVar &other) const
{
	return val.equals(other.val, type);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s) :
    id(p),
    val(v),
    type(t),
    sortId(s)
{
	GVar::all.push_back(this);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet) :
    id(p),
    val(v),
    type(t),
    sortId(s),
    set(aSet)
{
	GVar::all.push_back(this);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s, uint32_t flags) :
    id(p),
    val(v),
    type(t),
    sortId(s),
    flags(flags)
{
	GVar::all.push_back(this);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s, GVar_Set aSet, uint32_t flags) :
    id(p),
    val(v),
    type(t),
    sortId(s),
    set(aSet),
    flags(flags)
{
	GVar::all.push_back(this);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list) :
    id(p),
    val(v),
    type(t),
    sortId(s),
    set({list})
{
	GVar::all.push_back(this);
}

GVar::GVar(std::string p, GVar_Val v, GVar_Type t, int s, std::vector<std::string> list, uint32_t flags) :
    id(p),
    val(v),
    type(t),
    sortId(s),
    set({list}),
    flags(flags)
{
	GVar::all.push_back(this);
}


void GVar::writeToJson(json &j)
{
	switch (type)
	{
		case GVAR_EVENT:
			/*j[id] = val.v_bool;*/
			break;
		case GVAR_BOOL:
			j[id] = val.v_bool;
			break;
		case GVAR_FLOAT:
			j[id] = val.v_float;
			break;
		case GVAR_UNORM:
			j[id] = val.v_float;
			break;
		case GVAR_UINT:
			j[id] = val.v_uint;
			break;
		case GVAR_INT:
			j[id] = val.v_int;
			break;
		case GVAR_VEC3:
			j[id] = {val.v_vec3[0], val.v_vec3[1], val.v_vec3[2]};
			break;
		case GVAR_DISPLAY_FLOAT:
			j[id] = val.v_float;
			break;
		case GVAR_ENUM:
			j[id] = val.v_int;
			break;
		case GVAR_UINT_RANGE:
			j[id] = val.v_uint;
			break;
		case GVAR_FLOAT_RANGE:
			j[id] = val.v_float;
			break;
		case GVAR_VEC3_RANGE:
			j[id] = {val.v_vec3[0], val.v_vec3[1], val.v_vec3[2]};
			break;
		case GVAR_FILE_INPUT:
			j[id] = std::string(val.v_char_array.data());
			break;
		case GVAR_FILE_OUTPUT:
			j[id] = std::string(val.v_char_array.data());
			break;
		case GVAR_DISPLAY_TEXT:
			j[id] = std::string(val.v_char_array.data());
			break;
		default:
			// DEBUG_BREAK;
			break;
	}
}

void GVar::readFromJson(json &j)
{
	if (j.find(id) == j.end())
	{
		return;
	}
	if ((flags & GVAR_FLAGS_NO_LOAD) != 0)
	{
		return;
	}
	std::string str;
	switch (type)
	{
		case GVAR_EVENT:
			/*val.v_bool = j[id];*/
			break;
		case GVAR_BOOL:
			val.v_bool = j[id];
			break;
		case GVAR_FLOAT:
			val.v_float = j[id];
			break;
		case GVAR_UNORM:
			val.v_float = j[id];
			break;
		case GVAR_UINT:
			val.v_uint = j[id];
			break;
		case GVAR_INT:
			val.v_int = j[id];
			break;
		case GVAR_VEC3:
			val.v_vec3[0] = j[id][0];
			val.v_vec3[1] = j[id][1];
			val.v_vec3[2] = j[id][2];
			break;
		case GVAR_DISPLAY_FLOAT:
			//val.v_float = j[id];
			break;
		case GVAR_ENUM:
			val.v_int  = j[id];
			val.v_uint = val.v_int;
			break;
		case GVAR_UINT_RANGE:
			val.v_uint = j[id];
			break;
		case GVAR_FLOAT_RANGE:
			val.v_float = j[id];
			break;
		case GVAR_VEC3_RANGE:
			val.v_vec3[0] = j[id][0];
			val.v_vec3[1] = j[id][1];
			val.v_vec3[2] = j[id][2];
			break;
		case GVAR_FILE_INPUT:
			str = j[id];
			val.v_char_array = std::vector<char>(str.begin(), str.end());
			if (val.v_char_array.back() != '\0')
			{
				val.v_char_array.push_back('\0');
			}
			break;
		case GVAR_FILE_OUTPUT:
			str              = j[id];
			val.v_char_array = std::vector<char>(str.begin(), str.end());
			if (val.v_char_array.back() != '\0')
			{
				val.v_char_array.push_back('\0');
			}
			break;
		case GVAR_DISPLAY_TEXT:
			str              = j[id];
			val.v_char_array = std::vector<char>(str.begin(), str.end());
			if (val.v_char_array.empty() || val.v_char_array.back() != '\0')
			{
				val.v_char_array.push_back('\0');
			}
		default:
			// DEBUG_BREAK;
			break;
	}
}

bool GVar::addToGui(uint32_t guiFlags)
{
	std::stringstream ss;
	GVar_Val oldVal = val;
	GVar_Val newVal;
	std::string       str, res;
	bool internalEvent = false;
	switch (type)
	{
		case GVAR_EVENT:
			val.v_bool = ImGui::Button(id.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(id.c_str(), &val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(id.c_str(), ImGuiDataType_Float, &val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(id.c_str(), &val.v_float, 0.0f, 1.0f);
			break;
		case GVAR_UINT:
			ImGui::InputScalar(id.c_str(), ImGuiDataType_U32, &val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(id.c_str(), &val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(id.c_str(), val.v_vec3);
			break;
		case GVAR_VEC3_RANGE:
			ImGui::SliderFloat3(id.c_str(), val.v_vec3, set.range.min.v_float, set.range.max.v_float);
			break;
		case GVAR_DISPLAY_FLOAT:
			ImGui::Text(id.c_str(), val.v_float);
			break;
		case GVAR_DISPLAY_UINT:
			ImGui::Text(id.c_str(), val.v_uint);
			break;
		case GVAR_DISPLAY_TEXT:
			ImGui::Text(id.c_str());
			ImGui::TextWrapped(val.v_char_array.data());
			break;
		case GVAR_ENUM:
			if (guiFlags & GUI_FLAGS_MENU_BAR)
			{
				if (flags & GVAR_FLAGS_V2)
				{
					for (size_t i = 0; i < set.list.size(); i++)
					{
						if (ImGui::MenuItem(set.list[i].c_str()))
						{
							val.v_int = i;
							val.v_uint = val.v_int;
						}
					}
				}
				else
				{
					if (ImGui::BeginMenu(id.c_str()))
					{
						for (size_t i = 0; i < set.list.size(); i++)
						{
							if (ImGui::MenuItem(set.list[i].c_str()))
							{
								val.v_int  = i;
								val.v_uint = val.v_int;
							}
						}
						ImGui::EndMenu();
					}
				}
			}
			else
			{
				for (size_t i = 0; i < set.list.size(); i++)
				{
					ss << set.list[i] << '\0';
				}
				ImGui::Combo(id.c_str(), &val.v_int, ss.str().c_str(), 5);
			}
			val.v_uint = val.v_int;
			break;
		case GVAR_UINT_RANGE:
			ImGui::SliderScalar(id.c_str(), ImGuiDataType_U32, &val.v_uint, &set.range.min.v_uint, &set.range.max.v_uint);
			break;
		case GVAR_FLOAT_RANGE:
			ImGui::SliderScalar(id.c_str(), ImGuiDataType_Float, &val.v_float, &set.range.min.v_float, &set.range.max.v_float);
			break;
		case GVAR_TEXT_INPUT:
			ImGui::InputTextMultiline(id.c_str(), val.v_char_array.data(), val.v_char_array.size());
			break;
		case GVAR_DISPLAY_UNORM:
			ImGui::Text(id.c_str());
			ImGui::ProgressBar(val.v_float, ImVec2(0.0f, 0.0f));
			break;
		case GVAR_FILE_INPUT:
			internalEvent = ImGui::Button(id.c_str());
			val.v_bool    = false;
			ImGui::TextWrapped(val.v_char_array.data());
			if (internalEvent)
			{
				val.v_focus = true; 
				if (set.list.size() > 0 && set.list[0] != "")
				{
					str = set.list[0] + " {" + set.list[0] + "},.*";
				}
				else
				{
					str = "";
				}
				if (set.list.size() > 1)
				{
					ifd::FileDialog::Instance().Open(id.c_str(), id.c_str(), str, false, set.list[1]);
				}
				else
				{
					ifd::FileDialog::Instance().Open(id.c_str(), id.c_str(), str, false);
				}
			}
			if (ifd::FileDialog::Instance().IsDone(id.c_str()))
			{
				val.v_focus = false;
				if (ifd::FileDialog::Instance().HasResult())
				{
					val.v_bool       = true;
					std::string res  = ifd::FileDialog::Instance().GetResult().u8string();
					val.v_char_array = std::vector<char>(res.begin(), res.end());
					if (val.v_char_array.back() != '\0')
					{
						val.v_char_array.push_back('\0');
					}
				}
				ifd::FileDialog::Instance().Close();
			}
			break;
		case GVAR_FILE_OUTPUT:
			internalEvent = ImGui::Button(id.c_str());
			val.v_bool    = false;
			ImGui::TextWrapped(val.v_char_array.data());
			if (internalEvent)
			{
				val.v_focus = true; 
				if (set.list.size() > 0 && set.list[0] != "")
				{
					str = set.list[0] + " {" + set.list[0] + "},.*";
				}
				else
				{
					str = "";
				}
				if (set.list.size() > 1)
				{
					ifd::FileDialog::Instance().Save(id.c_str(), id.c_str(), str, set.list[1]);
				}
				else
				{
					ifd::FileDialog::Instance().Save(id.c_str(), id.c_str(), str);
				}
			}
			if (ifd::FileDialog::Instance().IsDone(id.c_str()))
			{
				val.v_focus = false;
				if (ifd::FileDialog::Instance().HasResult())
				{
					val.v_bool       = true;
					std::string res  = ifd::FileDialog::Instance().GetResult().u8string();
					val.v_char_array = std::vector<char>(res.begin(), res.end());
					if (val.v_char_array.back() != '\0')
					{
						val.v_char_array.push_back('\0');
					}
				}
				ifd::FileDialog::Instance().Close();
			}
			break;
		default:
			break;
	}
	return !oldVal.equals(val, type);
}

bool GVar::holdsFocus()
{
	for (auto gv : all)
	{
		if (gv->val.v_focus)
		{
			return true;
		}
	}
	return false;
}

std::vector<GVar *> GVar::filterMask(std::vector<GVar *> gvar, uint32_t mask)
{
	std::vector<GVar *> result;
	for (auto gv : gvar)
	{
		if ((gv->flags & mask) != 0)
		{
			result.push_back(gv);
		}
	}
	return result;
}

std::vector<GVar *> GVar::filterSortID(std::vector<GVar *> gvar, uint32_t sortID)
{
	std::vector<GVar *> result;
	for (auto gv : gvar)
	{
		if (gv->sortId == sortID)
		{
			result.push_back(gv);
		}
	}
	return result;
}

bool GVar::addToGui(std::vector<GVar *> gvar, std::string category, uint32_t guiFlags)
{
	std::sort(gvar.begin(), gvar.end(), [](GVar *a, GVar *b) { return a->sortId < b->sortId; });
	bool changed = false;
	for (GVar* gv : gvar)
	{
		changed = gv->addToGui(guiFlags) || changed;
	}
	return changed;
}

void GVar::store(std::vector<GVar *> gvar, std::string path)
{
	json          j;
	std::ofstream o(path);
	for (auto gv : gvar)
	{
		gv->writeToJson(j);
	}
	o << std::setw(4) << j << std::endl;
}
void GVar::load(std::vector<GVar *> gvar, std::string path)
{
	json          j;
	std::ifstream i(path);
	if (!i.is_open())
	{
		return;
	}
	i >> j;
	for (auto gv : gvar)
	{
		gv->readFromJson(j);
	}
}
std::vector<GVar *> GVar::getAll()
{
	return all;
}

void GVar::loadAll(std::string path)
{
	load(all, path);
}

void GVar::storeAll(std::string path)
{
	store(all, path);
}

}        // namespace vka