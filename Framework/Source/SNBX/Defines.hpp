#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <span>
#include <glm/glm.hpp>
#include <unordered_map>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

#if defined(_DEBUG)
#define SNBX_DEBUG
#endif

#ifdef NDEBUG
#  define SNBX_ASSERT(condition, message) ((void)0)
#else
#  include <cassert>
#  define SNBX_ASSERT(condition, message) assert(condition && message)
#endif

#if _WIN64
#define SNBX_API __declspec(dllexport)
#define SNBX_WIN 1
#define SNBX_DESKTOP 1
#elif __linux__
#define SNBX_API
#define SNBX_LINUX 1
#define SNBX_DESKTOP 1
#endif

#ifndef SNBX_PRETTY_FUNCTION
#   if defined _MSC_VER
#      define SNBX_PRETTY_FUNCTION __FUNCSIG__
#   else
#       if defined __GNUC__
#           define SNBX_PRETTY_FUNCTION __PRETTY_FUNCTION__
#       else
static_assert(false, "OS still not supported");
#       endif
#   endif
#endif

using StringView = std::string_view;
using String = std::string;

template<typename T>
using Array = std::vector<T>;

template<typename T>
using Span = std::span<T>;

template<typename T, std::size_t N>
using FixedArray = std::array<T, N>;

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

using Path = fs::path;
using DirIterator = fs::directory_iterator;
using DirEntry = fs::directory_entry;

using Vec2 = glm::vec2;
using UVec2 = glm::uvec2;
using Vec3 = glm::vec3;
using UVec3 = glm::uvec3;
using Vec4 = glm::vec4;
using Quat = glm::quat;
using Mat4x4 = glm::mat4x4;

#define U32_MAX std::numeric_limits<u32>::max()

typedef decltype(sizeof(bool)) usize;

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef unsigned long int   ul32;

typedef signed char         i8;
typedef signed short        i16;
typedef signed int          i32;
typedef signed long long    i64;

typedef float   f32;
typedef double  f64;

typedef void      * CPtr;
typedef const void* ConstCPtr;
using TypeID = u64;

namespace SNBX
{

	constexpr static usize HashString(const char* str)
	{
		usize hash = 0;
		for (i32 i = 0;; ++i)
		{
			if (str[i] == 0) break;
			hash = str[i] + (hash << 6) + (hash << 16) - hash;
		}
		return hash;
	}

	template<typename T>
	constexpr static TypeID GetTypeId()
	{
		constexpr TypeID typeId = HashString(SNBX_PRETTY_FUNCTION);
		return typeId;
	}
}