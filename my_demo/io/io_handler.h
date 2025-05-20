#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include <stdbool.h>
#include <stdint.h>

/* Platform-specific configuration */
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #define NCDIAG_ENUM_HEADER "ncdiag_enums.h"
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
    #define NCDIAG_ENUM_HEADER <notcurses/ncdiag_enums.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #define NCDIAG_ENUM_HEADER <notcurses/ncdiag_enums.h>
#else
    #define PLATFORM_UNKNOWN
    #define NCDIAG_ENUM_HEADER <notcurses/ncdiag_enums.h>
#endif

/* Special handling for Windows */
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #define WIN32_LEAN_AND_MEAN
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif

/* Include notcurses */
#include <notcurses/notcurses.h>

/**
 * Operating system type enumeration
 */
typedef enum platform_type {
    PLATFORM_TYPE_UNKNOWN,
    PLATFORM_TYPE_WINDOWS,
    PLATFORM_TYPE_MACOS,
    PLATFORM_TYPE_LINUX
} platform_type_t;

/**
 * Main IO handler structure for Dungeon Crawl
 * Encapsulates the notcurses context and provides interface to input/output handlers
 */
typedef struct io_handler {
    struct notcurses* nc;        // Main notcurses context
    struct ncplane* stdplane;    // Standard plane for basic operations
    bool initialized;            // Flag to track initialization state
    platform_type_t platform;    // Current platform
    bool unicode_supported;      // Unicode supported flag
    char* base_path;             // Base path for resources
} io_handler_t;

/**
 * Initialize the IO handler with default settings
 * @return Initialized IO handler or NULL on failure
 */
io_handler_t* io_handler_init(void);

/**
 * Initialize the IO handler with custom options
 * @param flags Notcurses initialization flags
 * @return Initialized IO handler or NULL on failure
 */
io_handler_t* io_handler_init_with_flags(uint64_t flags);

/**
 * Render all changes to the terminal
 * @param handler Pointer to the IO handler
 * @return true on success, false on failure
 */
bool io_handler_render(io_handler_t* handler);

/**
 * Free all resources associated with the IO handler
 * @param handler Pointer to the IO handler
 */
void io_handler_cleanup(io_handler_t* handler);

/**
 * Get current platform type
 * @return Platform type enumeration
 */
platform_type_t io_handler_get_platform(void);

/**
 * Check if the terminal supports unicode
 * @param handler Pointer to the IO handler
 * @return true if unicode is supported, false otherwise
 */
bool io_handler_supports_unicode(io_handler_t* handler);

/**
 * Set the base path for loading resources
 * @param handler Pointer to the IO handler
 * @param path Base path for resources
 * @return true on success, false on failure
 */
bool io_handler_set_base_path(io_handler_t* handler, const char* path);

/**
 * Get full path for a resource file
 * @param handler Pointer to the IO handler
 * @param resource_path Relative path to the resource
 * @param full_path Buffer to store full path
 * @param max_len Maximum length of the buffer
 * @return true on success, false on failure
 */
bool io_handler_get_resource_path(io_handler_t* handler, const char* resource_path, 
                                char* full_path, size_t max_len);

/**
 * Check if a file exists
 * @param path Path to the file
 * @return true if file exists, false otherwise
 */
bool io_handler_file_exists(const char* path);

#endif /* IO_HANDLER_H */