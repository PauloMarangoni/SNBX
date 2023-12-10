cmake_minimum_required(VERSION 3.27)
project(sandbox)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/D3D12)

file(GLOB_RECURSE SANDBOX_SOURCES src/*.hpp src/*.cpp src/*.h src/*.c)

add_executable(sandbox ${SANDBOX_SOURCES})

target_link_libraries(sandbox PUBLIC spdlog)
target_link_libraries(sandbox PUBLIC glfw)

if (WIN32)
    target_link_libraries(sandbox PRIVATE D3D12Libs)
    target_link_libraries(sandbox PRIVATE d3d12 dxgi dxguid)
endif ()

target_include_directories(sandbox PUBLIC src)
target_include_directories(sandbox PUBLIC thirdparty/glm)