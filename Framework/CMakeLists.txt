add_subdirectory(Thirdparty/spdlog)
add_subdirectory(Thirdparty/glfw)
add_subdirectory(Thirdparty/D3D12)
add_subdirectory(Thirdparty/vulkan)
add_subdirectory(Thirdparty/SPIRV-Cross)


file(GLOB_RECURSE SNBX_SOURCES Source/*.hpp Source/*.cpp Source/*.h Source/*.c)

file(GLOB_RECURSE THIRDPARTY_SOURCES
				Thirdparty/GL/glad/src/glad.c
)

add_library(SNBX SHARED ${SNBX_SOURCES} ${THIRDPARTY_SOURCES})

target_include_directories(SNBX PUBLIC Source)

target_link_libraries(SNBX PUBLIC spdlog)
target_link_libraries(SNBX PRIVATE glfw)
target_link_libraries(SNBX PRIVATE vulkan-lib)
target_link_libraries(SNBX PRIVATE spdlog)
target_link_libraries(SNBX PRIVATE glfw)
target_link_libraries(SNBX PRIVATE vulkan-lib)
target_link_libraries(SNBX PRIVATE SPIRV-Cross)


if (WIN32)
	target_link_libraries(SNBX PRIVATE D3D12Libs)
	target_link_libraries(SNBX PRIVATE d3d12 dxgi dxguid)
endif ()

target_include_directories(SNBX PUBLIC Thirdparty/glm)
target_include_directories(SNBX PRIVATE Thirdparty/dxc/include)
target_include_directories(SNBX PRIVATE Thirdparty/GL/glad/include)



add_binary_file(Thirdparty/dxc dxcompiler)