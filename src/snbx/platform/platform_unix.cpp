#include "platform.hpp"
#if defined(SNBX_LINUX)

#include <dlfcn.h>

void* platform_load_shared_lib(const char* path) {
    char buffer[256];
    usize i = 0;
    buffer[i++] = 'l';
    buffer[i++] = 'i';
    buffer[i++] = 'b';
    for (; i < strlen(path) + 3; ++i) {
        buffer[i] = path[i - 3];
    }
    buffer[i++] = '.';
#ifdef SNBX_API
    buffer[i++] = 's';
    buffer[i++] = 'o';
#elif SNBX_MACOS
    buffer[i++] = 'd';
    buffer[i++] = 'y';
    buffer[i++] = 'l';
    buffer[i++] = 'i';
    buffer[i++] = 'b';
#endif
    buffer[i++] = '\0';

    void* ptr = dlopen(buffer, RTLD_NOW);
    if (!ptr) {
        const char* err = dlerror();
        spdlog::error("cannot open {}", err);
    }
    return ptr;
}

void* platform_get_proc_address(void* lib, const char* func_name) {
    return dlsym(lib, func_name);
}

#endif
