#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "io_handler.h"
#include "output/output_handler.h"
#include "input/input_handler.h"

// For cross-platform file existence checking
#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <direct.h>
#define F_OK 0
#define access _access
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

io_handler_t* io_handler_init(void) {
    return io_handler_init_with_flags(0);
}

platform_type_t io_handler_get_platform(void) {
#ifdef PLATFORM_WINDOWS
    return PLATFORM_TYPE_WINDOWS;
#elif defined(PLATFORM_MACOS)
    return PLATFORM_TYPE_MACOS;
#elif defined(PLATFORM_LINUX)
    return PLATFORM_TYPE_LINUX;
#else
    return PLATFORM_TYPE_UNKNOWN;
#endif
}

io_handler_t* io_handler_init_with_flags(uint64_t flags) {
    io_handler_t* handler = malloc(sizeof(io_handler_t));
    if (!handler) {
        fprintf(stderr, "Failed to allocate memory for IO handler\n");
        return NULL;
    }

    // Set default values
    handler->initialized = false;
    handler->nc = NULL;
    handler->stdplane = NULL;
    handler->platform = io_handler_get_platform();
    handler->unicode_supported = false;
    handler->base_path = NULL;

    // Initialize notcurses with provided flags
    struct notcurses_options opts = {
        .flags = flags,
        .margin_t = 0,
        .margin_r = 0,
        .margin_b = 0,
        .margin_l = 0
    };

    // Detect and handle environment
    char* term = getenv("TERM");
    opts.termtype = term;

#ifdef PLATFORM_WINDOWS
    // On Windows, suppress legacy terminal modes if possible
    flags |= NCOPTION_NO_WINCH_SIGHANDLER;
#endif

    handler->nc = notcurses_init(&opts, stdout);
    if (!handler->nc) {
        fprintf(stderr, "Failed to initialize notcurses\n");
        free(handler);
        return NULL;
    }

    // Get standard plane for basic operations
    handler->stdplane = notcurses_stdplane(handler->nc);
    if (!handler->stdplane) {
        fprintf(stderr, "Failed to get standard plane\n");
        notcurses_stop(handler->nc);
        free(handler);
        return NULL;
    }

    // Check if Unicode is supported
    handler->unicode_supported = notcurses_canutf8(handler->nc);

    // Initialize output subsystems
    if (!output_handler_init(handler)) {
        fprintf(stderr, "Failed to initialize output handler\n");
        notcurses_stop(handler->nc);
        free(handler);
        return NULL;
    }

    // By default, set the base path to the current directory
    io_handler_set_base_path(handler, ".");

    handler->initialized = true;
    return handler;
}

bool io_handler_render(io_handler_t* handler) {
    if (!handler || !handler->initialized || !handler->nc) {
        return false;
    }

    // Render all changes to the terminal
    int ret = notcurses_render(handler->nc);
    return ret == 0;
}

void io_handler_cleanup(io_handler_t* handler) {
    if (!handler) {
        return;
    }

    if (handler->nc) {
        // Cleanup output subsystems first
        output_handler_cleanup(handler);
        
        // Stop notcurses
        notcurses_stop(handler->nc);
    }

    // Free base path
    if (handler->base_path) {
        free(handler->base_path);
    }

    free(handler);
}

bool io_handler_supports_unicode(io_handler_t* handler) {
    if (!handler || !handler->nc) {
        return false;
    }
    
    return handler->unicode_supported;
}

bool io_handler_set_base_path(io_handler_t* handler, const char* path) {
    if (!handler || !path) {
        return false;
    }
    
    // Free any existing base path
    if (handler->base_path) {
        free(handler->base_path);
        handler->base_path = NULL;
    }
    
    // Allocate and copy the new path
    handler->base_path = strdup(path);
    if (!handler->base_path) {
        return false;
    }
    
    // Remove trailing slash if present
    size_t len = strlen(handler->base_path);
    if (len > 0 && (handler->base_path[len - 1] == '/' || 
                    handler->base_path[len - 1] == '\\')) {
        handler->base_path[len - 1] = '\0';
    }
    
    return true;
}

bool io_handler_get_resource_path(io_handler_t* handler, const char* resource_path, 
                                char* full_path, size_t max_len) {
    if (!handler || !handler->base_path || !resource_path || !full_path || max_len < 2) {
        return false;
    }
    
    // Normalize resource path (remove leading slash if present)
    const char* normalized_path = resource_path;
    if (resource_path[0] == '/' || resource_path[0] == '\\') {
        normalized_path = resource_path + 1;
    }
    
    // Calculate required length
    size_t base_len = strlen(handler->base_path);
    size_t res_len = strlen(normalized_path);
    
    // Check if the combined path fits in the buffer
    if (base_len + res_len + 2 > max_len) {  // +2 for separator and null terminator
        return false;
    }
    
    // Combine paths
    strcpy(full_path, handler->base_path);
    full_path[base_len] = PATH_SEPARATOR;
    strcpy(full_path + base_len + 1, normalized_path);
    
    // Convert separators to platform-specific ones
#ifdef PLATFORM_WINDOWS
    // On Windows, convert forward slashes to backslashes
    for (size_t i = 0; i < strlen(full_path); i++) {
        if (full_path[i] == '/') {
            full_path[i] = '\\';
        }
    }
#else
    // On Unix systems, convert backslashes to forward slashes
    for (size_t i = 0; i < strlen(full_path); i++) {
        if (full_path[i] == '\\') {
            full_path[i] = '/';
        }
    }
#endif
    
    return true;
}

bool io_handler_file_exists(const char* path) {
    if (!path) {
        return false;
    }
    
    // Check if file exists
    return access(path, F_OK) == 0;
}