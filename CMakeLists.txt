cmake_minimum_required(VERSION 3.27)
project(SNBX)

set(CMAKE_CXX_STANDARD 20)

include(cmake/CMakeFuncs.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
    add_compile_options(-fms-extensions)
endif()


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

if ( MSVC )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
endif (MSVC)


if (UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS_INIT} -fPIC")
    set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS_INIT} -fPIC")
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_INSTALL_RPATH "\${ORIGIN}")
elseif (APPLE)
    set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS_INIT} -fPIC")
    set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS_INIT} -fPIC")
    SET(CMAKE_SKIP_BUILD_RPATH FALSE)
    #SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_INSTALL_RPATH "${ORIGIN}/")
endif()

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
target_include_directories(SNBX PRIVATE thirdparty/dxc/include)

add_binary_file(thirdparty/dxc dxcompiler)