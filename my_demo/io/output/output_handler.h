#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <notcurses/notcurses.h>
#include "../io_handler.h"

/**
 * Common color definitions for easy usage
 */
typedef enum output_color {
    COLOR_DEFAULT = 0,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
} output_color_t;

/**
 * Output handler structure for managing planes and colors
 */
typedef struct output_handler {
    struct ncplane* game_plane;    // Main game plane
    struct ncplane* ui_plane;      // UI elements plane
    struct ncplane* message_plane; // Messages/log plane
    uint64_t default_channels;     // Default channels configuration
} output_handler_t;

/**
 * Get the singleton output handler instance
 * @return Pointer to the output handler instance
 */
output_handler_t* output_handler_get(void);

/**
 * Initialize the output handler
 * @param io_handler Pointer to the parent IO handler
 * @return true on success, false on failure
 */
bool output_handler_init(io_handler_t* io_handler);

/**
 * Clean up the output handler resources
 * @param io_handler Pointer to the parent IO handler
 */
void output_handler_cleanup(io_handler_t* io_handler);

/**
 * Create a new plane with specified dimensions and position
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param rows Number of rows
 * @param cols Number of columns
 * @param z_index Z-index for plane stacking (higher = on top)
 * @return Pointer to the created plane or NULL on failure
 */
struct ncplane* output_handler_create_plane(int y, int x, int rows, int cols, int z_index);

/**
 * Get foreground RGB channel from predefined color
 * @param color Predefined color
 * @param channels Pointer to channel variable to modify
 */
void output_handler_set_fg_color(output_color_t color, uint64_t* channels);

/**
 * Get background RGB channel from predefined color
 * @param color Predefined color
 * @param channels Pointer to channel variable to modify
 */
void output_handler_set_bg_color(output_color_t color, uint64_t* channels);

/**
 * Set RGB foreground color directly
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param channels Pointer to channel variable to modify
 */
void output_handler_set_fg_rgb(uint8_t r, uint8_t g, uint8_t b, uint64_t* channels);

/**
 * Set RGB background color directly
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param channels Pointer to channel variable to modify
 */
void output_handler_set_bg_rgb(uint8_t r, uint8_t g, uint8_t b, uint64_t* channels);

/**
 * Clear a plane with specified background color
 * @param plane Plane to clear
 * @param color Background color to use
 * @return true on success, false on failure
 */
bool output_handler_clear_plane(struct ncplane* plane, output_color_t color);

/**
 * Get the screen dimensions
 * @param rows Pointer to store row count
 * @param cols Pointer to store column count
 */
void output_handler_get_screen_size(unsigned* rows, unsigned* cols);

#endif /* OUTPUT_HANDLER_H */