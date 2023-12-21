if (WIN32)
    add_library(D3D12Libs src/D3D12MemAlloc.cpp)
    target_include_directories(D3D12Libs PUBLIC include)
endif()