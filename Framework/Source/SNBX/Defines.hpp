#pragma once

#include <string_view>
#include <string>
#include <vector>
#include <span>
#include <glm/glm.hpp>

#if defined(_DEBUG)
#define SNBX_DEBUG
#endif

#ifdef NDEBUG
#  define SNBX_ASSERT(condition, message) ((void)0)
#else
#  include <cassert>
#include <unordered_map>
#include <filesystem>
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


using StringView = std::string_view;
using String = std::string;

template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Span = std::span<T>;

template<typename T, std::size_t N>
using Array = std::array<T, N>;

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;

using Path = std::filesystem::path;

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
