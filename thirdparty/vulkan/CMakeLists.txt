add_library(vulkan-lib STATIC
				spirv-reflect/spirv_reflect.c
				volk/volk.c
				)

target_include_directories(vulkan-lib PUBLIC volk)
target_include_directories(vulkan-lib PUBLIC vma)
target_include_directories(vulkan-lib PUBLIC vulkan/include)
target_include_directories(vulkan-lib PUBLIC spirv-reflect)