
# warning options
option(SPDLOG_BUILD_WARNINGS "Enable compiler warnings" OFF)


file(GLOB_RECURSE SPD_SOURCES src/*.hpp src/*.cpp src/*.h src/*.c)
file(GLOB_RECURSE SPD_INCLUDES src/*.hpp src/*.cpp src/*.h src/*.c)
add_library(spdlog STATIC ${SPD_SOURCES} ${SPD_INCLUDES})

target_compile_definitions(spdlog PUBLIC SPDLOG_USE_STD_FORMAT)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)
target_include_directories(spdlog PUBLIC include)