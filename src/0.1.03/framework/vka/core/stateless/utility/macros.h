#pragma once
#include <iostream>
#include <vulkan/vulkan.h>
#include "error_string.h"

#ifndef API_VERSION
#	define API_VERSION VK_API_VERSION_1_3
#endif        // !1

#ifndef ENGINE_NAME
#	define ENGINE_NAME "vka"
#endif        // !1

#ifndef ENGINE_VERSION
#	define ENGINE_VERSION VK_MAKE_VERSION(0, 1, 3)
#endif        // !1

#define VALIDATION true

#ifdef _WIN32
#	define GLSLC_COMMAND "glslc.exe --target-env=vulkan1.2"
#else        // not _WIN32
#	define GLSLC_COMMAND "glslc --target-env vulkan1.2"
#endif        // _WIN32

#define INTEL_VENDOR_ID 8086
#define AMD_VENDOR_ID 0x1002
#define NVIDIA_VENDOR_ID 0x10de

#ifdef _WIN32
#	define FILE_SEPARATOR "\\"

#else        // not _WIN32
#	include <signal.h>
#	define FILE_SEPARATOR '/'
#	define DEBUG_BREAK raise(SIGTRAP);

#endif        // _WIN32

#define DELETE_COPY_CONSTRUCTORS(A)   \
	A(const A &)            = delete; \
	A(A &&)                 = delete; \
	A &operator=(const A &) = delete; \
	A &operator=(A &&)      = delete;

#define VKA_COUNT(container) static_cast<uint32_t>((container).size())

#ifdef _MSC_VER
#	define DEBUG_BREAK __debugbreak();
#else
#	define DEBUG_BREAKDebugBreak() ;
#endif

#ifndef VALIDATION
#	define VALIDATION true
#endif        // !VALIDATION

#ifdef _DEBUG
// In debug mode validation should always be on
#	undef VALIDATION
#	define VALIDATION true
#endif

#if VALIDATION
#	define VK_CHECK(val)                                                                                                             \
		{                                                                                                                             \
			VkResult res = (val);                                                                                                     \
			if (res != VK_SUCCESS)                                                                                                    \
			{                                                                                                                         \
				std::cout << "Fatal : VkResult is \"" << errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
				DEBUG_BREAK                                                                                                           \
			}                                                                                                                         \
		}
#	define VKA_CHECK(val)                                                                               \
		{                                                                                                \
			if (!(val))                                                                                  \
			{                                                                                            \
				std::cout << "Fatal : Assert failed in " << __FILE__ << " at line " << __LINE__ << "\n"; \
				DEBUG_BREAK                                                                              \
			}                                                                                            \
		}
#	define IF_VALIDATION(f) f
// like check true but does not execute statement if validation is turned off
#	define VKA_ASSERT(val)                                                                              \
		{                                                                                                \
			if (!(val))                                                                                  \
			{                                                                                            \
				std::cout << "Fatal : Assert failed in " << __FILE__ << " at line " << __LINE__ << "\n"; \
				DEBUG_BREAK                                                                              \
			}                                                                                            \
		}
#else
#	define VK_CHECK(val) val
#	define CHECK_TRUE(val) val
#	define IF_VALIDATION(f)
#	define VKA_ASSERT(val)
#endif        // _DEBUG

// Get function pointers for vulkan commands from extensions.
// Makes function available as p<function name>
#define LOAD_CMD_VK_DEVICE(FUNCTION_NAME, DEVICE)                                                             \
	PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) vkGetDeviceProcAddr(DEVICE, #FUNCTION_NAME); \
	VKA_CHECK(p##FUNCTION_NAME != nullptr)
#define LOAD_CMD_VK_INSTANCE(FUNCTION_NAME, INSTANCE)                                                             \
	PFN_##FUNCTION_NAME p##FUNCTION_NAME = (PFN_##FUNCTION_NAME) vkGetInstanceProcAddr(INSTANCE, #FUNCTION_NAME); \
	VKA_CHECK(p##FUNCTION_NAME != nullptr)

#define NEXT_INDEX(x, y) ((x + 1) % (y))
#define PREVIOUS_INDEX(x, y) (((x + y - 1) % y))

#define DEFINE_ZERO_PAD                         \
	(TYPE) struct TYPE##_ZERO_PAD : public TYPE \
	{                                           \
		TYPE##_ZERO_PAD()                       \
		{                                       \
			memset(this, 0, sizeof(TYPE));      \
		}                                       \
	};
#define ZERO_PAD(TYPE) TYPE##_ZERO_PAD

#define DECLARE_HASH(TYPE, FUNC)                  \
	namespace std                                 \
	{                                             \
	template <>                                   \
	struct hash<TYPE>                             \
	{                                             \
		size_t operator()(TYPE const &type) const \
		{                                         \
			return type.FUNC();                   \
		}                                         \
	};                                            \
	}

#define DEFINE_EQUALS_OVERLOAD(CHILD_TYPE, PARENT_TYPE)             \
	bool CHILD_TYPE## ::operator==(const PARENT_TYPE & other) const \
	{                                                               \
		if (typeid(*this) == typeid(other))                         \
		{                                                           \
			return *this == static_cast<const CHILD_TYPE &>(other); \
		}                                                           \
		return false;                                               \
	}

#define DECLARE_HASH(TYPE, FUNC)                  \
	namespace std                                 \
	{                                             \
	template <>                                   \
	struct hash<TYPE>                             \
	{                                             \
		size_t operator()(TYPE const &type) const \
		{                                         \
			return type.##FUNC##();               \
		}                                         \
	};                                            \
	}