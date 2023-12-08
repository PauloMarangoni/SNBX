#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

using StringView = std::string_view;
using String = std::string;

template<typename T>
using Vector = std::vector<T>;

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value>;


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