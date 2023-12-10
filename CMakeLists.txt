cmake_minimum_required(VERSION 3.27)
project(SNBX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/D3D12)
add_subdirectory(thirdparty/vulkan)


file(GLOB_RECURSE SANDBOX_SOURCES src/*.hpp src/*.cpp src/*.h src/*.c)

add_executable(SNBX ${SANDBOX_SOURCES})

target_link_libraries(SNBX PUBLIC spdlog)
target_link_libraries(SNBX PUBLIC glfw)
target_link_libraries(SNBX PUBLIC vulkan-lib)

if (WIN32)
    target_link_libraries(SNBX PRIVATE D3D12Libs)
    target_link_libraries(SNBX PRIVATE d3d12 dxgi dxguid)
endif ()

target_include_directories(SNBX PUBLIC src)
target_include_directories(SNBX PUBLIC thirdparty/glm)