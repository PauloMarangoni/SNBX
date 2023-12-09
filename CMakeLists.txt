cmake_minimum_required(VERSION 3.27)
project(sandbox)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(thirdparty/spdlog)
add_subdirectory(Thirdparty/glfw)
add_subdirectory(Thirdparty/D3D12)

file(GLOB_RECURSE SANDBOX_SOURCES src/*.hpp src/*.cpp src/*.h src/*.c)

add_executable(sandbox ${SANDBOX_SOURCES})

target_link_libraries(sandbox PUBLIC spdlog)
target_link_libraries(sandbox PRIVATE glfw)
target_link_libraries(sandbox PRIVATE D3D12Libs)
target_link_libraries(sandbox PRIVATE d3d12 dxgi dxguid)

target_include_directories(sandbox PUBLIC src)