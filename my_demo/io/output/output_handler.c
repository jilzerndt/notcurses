#include <stdlib.h>
#include <stdio.h>
#include "output_handler.h"

// Singleton instance of the output handler
static output_handler_t* handler_instance = NULL;

// Store reference to the IO handler for cleanup
static io_handler_t* io_handler_ref = NULL;

// RGB color mapping for predefined colors
static const struct rgb_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} COLOR_MAP[] = {
    {255, 255, 255}, // COLOR_DEFAULT (white)
    {0, 0, 0},       // COLOR_BLACK
    {255, 0, 0},     // COLOR_RED
    {0, 255, 0},     // COLOR_GREEN
    {255, 255, 0},   // COLOR_YELLOW
    {0, 0, 255},     // COLOR_BLUE
    {255, 0, 255},   // COLOR_MAGENTA
    {0, 255, 255},   // COLOR_CYAN
    {255, 255, 255}  // COLOR_WHITE
};

output_handler_t* output_handler_get(void) {
    return handler_instance;
}

bool output_handler_init(io_handler_t* io_handler) {
    if (!io_handler || !io_handler->nc || !io_handler->stdplane) {
        return false;
    }

    io_handler_ref = io_handler;

    // Create singleton instance if not already created
    if (!handler_instance) {
        handler_instance = malloc(sizeof(output_handler_t));
        if (!handler_instance) {
            fprintf(stderr, "Failed to allocate memory for output handler\n");
            return false;
        }
    }

    // Initialize with default values
    handler_instance->default_channels = 0;
    
    // Initialize default channels
    output_handler_set_fg_color(COLOR_WHITE, &handler_instance->default_channels);
    output_handler_set_bg_color(COLOR_BLACK, &handler_instance->default_channels);

    // Get screen dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler->nc, &rows, &cols);

    // Create planes for different parts of the game
    // Game area (central area for main gameplay)
    handler_instance->game_plane = ncplane_create(io_handler->stdplane, &(struct ncplane_options){
        .y = 1,
        .x = 1,
        .rows = rows - 6,
        .cols = cols - 2,
        .name = "game",
        .resizecb = NULL,
        .flags = 0,
    });

    // UI area (top area for stats, health, etc.)
    handler_instance->ui_plane = ncplane_create(io_handler->stdplane, &(struct ncplane_options){
        .y = rows - 5,
        .x = 1,
        .rows = 3,
        .cols = cols - 2,
        .name = "ui",
        .resizecb = NULL,
        .flags = 0,
    });

    // Message area (bottom area for game messages)
    handler_instance->message_plane = ncplane_create(io_handler->stdplane, &(struct ncplane_options){
        .y = rows - 2,
        .x = 1,
        .rows = 1,
        .cols = cols - 2,
        .name = "messages",
        .resizecb = NULL,
        .flags = 0,
    });

    // Check if all planes were created successfully
    if (!handler_instance->game_plane || !handler_instance->ui_plane || !handler_instance->message_plane) {
        fprintf(stderr, "Failed to create game planes\n");
        output_handler_cleanup(io_handler);
        return false;
    }

    // Set default styles and colors for each plane
    ncplane_set_base(handler_instance->game_plane, " ", 0, handler_instance->default_channels);
    ncplane_set_base(handler_instance->ui_plane, " ", 0, handler_instance->default_channels);
    ncplane_set_base(handler_instance->message_plane, " ", 0, handler_instance->default_channels);

    return true;
}

void output_handler_cleanup(io_handler_t* io_handler) {
    if (!handler_instance) {
        return;
    }

    // Free all planes in reverse order of creation
    if (handler_instance->message_plane) {
        ncplane_destroy(handler_instance->message_plane);
    }
    
    if (handler_instance->ui_plane) {
        ncplane_destroy(handler_instance->ui_plane);
    }
    
    if (handler_instance->game_plane) {
        ncplane_destroy(handler_instance->game_plane);
    }

    // Free the handler instance
    free(handler_instance);
    handler_instance = NULL;
}

struct ncplane* output_handler_create_plane(int y, int x, int rows, int cols, int z_index) {
    if (!io_handler_ref || !io_handler_ref->nc || !handler_instance) {
        return NULL;
    }

    // Create a new plane with the specified parameters
    struct ncplane* new_plane = ncplane_create(io_handler_ref->stdplane, &(struct ncplane_options){
        .y = y,
        .x = x,
        .rows = rows,
        .cols = cols,
        .name = NULL, // Unnamed plane
        .resizecb = NULL,
        .flags = 0,
    });

    if (!new_plane) {
        return NULL;
    }

    // Set Z-index if specified (higher = on top)
    if (z_index > 0) {
        ncplane_move_above(new_plane, io_handler_ref->stdplane);
    }

    // Initialize with default style
    ncplane_set_base(new_plane, " ", 0, handler_instance->default_channels);

    return new_plane;
}

void output_handler_set_fg_color(output_color_t color, uint64_t* channels) {
    if (!channels || color >= sizeof(COLOR_MAP)/sizeof(COLOR_MAP[0])) {
        return;
    }

    struct rgb_color rgb = COLOR_MAP[color];
    NCCHANNELS_SET_FG_RGB8(channels, rgb.r, rgb.g, rgb.b);
}

void output_handler_set_bg_color(output_color_t color, uint64_t* channels) {
    if (!channels || color >= sizeof(COLOR_MAP)/sizeof(COLOR_MAP[0])) {
        return;
    }

    struct rgb_color rgb = COLOR_MAP[color];
    NCCHANNELS_SET_BG_RGB8(channels, rgb.r, rgb.g, rgb.b);
}

void output_handler_set_fg_rgb(uint8_t r, uint8_t g, uint8_t b, uint64_t* channels) {
    if (!channels) {
        return;
    }

    NCCHANNELS_SET_FG_RGB8(channels, r, g, b);
}

void output_handler_set_bg_rgb(uint8_t r, uint8_t g, uint8_t b, uint64_t* channels) {
    if (!channels) {
        return;
    }

    NCCHANNELS_SET_BG_RGB8(channels, r, g, b);
}

bool output_handler_clear_plane(struct ncplane* plane, output_color_t color) {
    if (!plane) {
        return false;
    }

    uint64_t channels = 0;
    output_handler_set_bg_color(color, &channels);
    return ncplane_set_base(plane, " ", 0, channels) != -1;
}

void output_handler_get_screen_size(unsigned* rows, unsigned* cols) {
    if (!io_handler_ref || !io_handler_ref->nc) {
        if (rows) *rows = 0;
        if (cols) *cols = 0;
        return;
    }

    notcurses_term_dim_yx(io_handler_ref->nc, rows, cols);
}

bool output_handler_render(void) {
    if (!io_handler_ref) {
        return false;
    }
    
    return io_handler_render(io_handler_ref);
}