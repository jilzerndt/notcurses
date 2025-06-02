#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../io_handler.h"
#include "text_output_handler.h"

// External reference to the IO handler
extern io_handler_t* io_handler_ref;

// Helper function to set style and colors on a plane
static void set_style_and_colors(struct ncplane* plane, output_color_t fg_color, 
                                output_color_t bg_color, text_style_t style) {
    if (!plane) {
        return;
    }

    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    ncplane_set_styles(plane, style);
    ncplane_set_channels(plane, channels);
}

// Helper function to convert text_align_t to ncalign_e
static ncalign_e convert_alignment(text_align_t align) {
    switch (align) {
        case TEXT_ALIGN_LEFT:
            return NCALIGN_LEFT;
        case TEXT_ALIGN_CENTER:
            return NCALIGN_CENTER;
        case TEXT_ALIGN_RIGHT:
            return NCALIGN_RIGHT;
        default:
            return NCALIGN_LEFT;
    }
}

// Helper function to set transparency level for an overlay plane
static void set_transparency_level(struct ncplane* plane, text_transparency_t transparency, 
                                  uint64_t* channels) {
    if (!plane || !channels) {
        return;
    }

    // Set foreground to fully visible
    NCCHANNELS_SET_FG_ALPHA(channels, NCALPHA_OPAQUE);

    // Set background transparency based on level
    switch (transparency) {
        case TEXT_TRANSPARENCY_NONE:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_OPAQUE);
            break;
        case TEXT_TRANSPARENCY_TEXT_ONLY:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_TRANSPARENT);
            break;
        case TEXT_TRANSPARENCY_BACKGROUND_LIGHT:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_HIGHCONTRAST);
            break;
        case TEXT_TRANSPARENCY_BACKGROUND_MEDIUM:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_BLEND);
            break;
        case TEXT_TRANSPARENCY_BACKGROUND_HEAVY:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_BLEND);
            // Additional darkening of background to achieve heavy transparency
            // We use a very dark color with medium alpha
            NCCHANNELS_SET_BG_RGB8(channels, 10, 10, 10);
            break;
        default:
            NCCHANNELS_SET_BG_ALPHA(channels, NCALPHA_TRANSPARENT);
            break;
    }
}

int text_output_print(struct ncplane* plane, int y, int x, 
                     output_color_t fg_color, output_color_t bg_color, 
                     text_style_t style, const char* format, ...) {
    if (!plane || !format) {
        return -1;
    }

    // Set up style and colors
    set_style_and_colors(plane, fg_color, bg_color, style);

    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print the text at the specified position
    return ncplane_putstr_yx(plane, y, x, buffer);
}

int text_output_print_game(int y, int x, output_color_t fg_color, 
                          output_color_t bg_color, text_style_t style, 
                          const char* format, ...) {
    output_handler_t* handler = output_handler_get();
    if (!handler || !handler->game_plane || !format) {
        return -1;
    }

    // Set up style and colors
    set_style_and_colors(handler->game_plane, fg_color, bg_color, style);

    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print the text at the specified position
    return ncplane_putstr_yx(handler->game_plane, y, x, buffer);
}

int text_output_print_ui(int y, int x, output_color_t fg_color, 
                        output_color_t bg_color, text_style_t style, 
                        const char* format, ...) {
    output_handler_t* handler = output_handler_get();
    if (!handler || !handler->ui_plane || !format) {
        return -1;
    }

    // Set up style and colors
    set_style_and_colors(handler->ui_plane, fg_color, bg_color, style);

    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print the text at the specified position
    return ncplane_putstr_yx(handler->ui_plane, y, x, buffer);
}

int text_output_print_message(output_color_t fg_color, output_color_t bg_color, 
                             text_style_t style, const char* format, ...) {
    output_handler_t* handler = output_handler_get();
    if (!handler || !handler->message_plane || !format) {
        return -1;
    }

    // Set up style and colors
    set_style_and_colors(handler->message_plane, fg_color, bg_color, style);

    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print the text at position 0, 0 of the message plane
    return ncplane_putstr_yx(handler->message_plane, 0, 0, buffer);
}

int text_output_print_aligned(struct ncplane* plane, int y, text_align_t align, 
                            output_color_t fg_color, output_color_t bg_color,
                            text_style_t style, const char* format, ...) {
    if (!plane || !format) {
        return -1;
    }

    // Set up style and colors
    set_style_and_colors(plane, fg_color, bg_color, style);

    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Calculate alignment position based on the plane width
    ncalign_e nc_align = convert_alignment(align);
    
    // Print the text with proper alignment
    return ncplane_putstr_aligned(plane, y, nc_align, buffer);
}

int text_output_clear_message(output_color_t fg_color, output_color_t bg_color, 
                            text_style_t style, const char* format, ...) {
    output_handler_t* handler = output_handler_get();
    if (!handler || !handler->message_plane || !format) {
        return -1;
    }

    // Clear the message plane
    uint64_t channels = 0;
    output_handler_set_bg_color(COLOR_BLACK, &channels);
    ncplane_set_base(handler->message_plane, " ", 0, channels);
    
    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Print the message
    return text_output_print_message(fg_color, bg_color, style, "%s", buffer);
}

struct ncplane* text_output_create_overlay(text_transparency_t transparency) {
    if (!io_handler_ref || !io_handler_ref->nc || !io_handler_ref->stdplane) {
        return NULL;
    }
    
    // Get the terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Create the overlay plane covering the full screen
    return text_output_create_overlay_sized(0, 0, rows, cols, transparency);
}

struct ncplane* text_output_create_overlay_sized(int y, int x, int rows, int cols, 
                                               text_transparency_t transparency) {
    if (!io_handler_ref || !io_handler_ref->nc || !io_handler_ref->stdplane) {
        return NULL;
    }
    
    // Create a transparent overlay plane
    struct ncplane* overlay = ncplane_create(io_handler_ref->stdplane, 
                                           &(struct ncplane_options){
                                              .y = y,
                                              .x = x,
                                              .rows = rows,
                                              .cols = cols,
                                              .name = "text_overlay",
                                              .resizecb = NULL,
                                              .flags = 0,
                                           });
    
    if (!overlay) {
        return NULL;
    }
    
    // Configure transparency for the overlay
    uint64_t channels = 0;
    
    // Set foreground to white by default
    NCCHANNELS_SET_FG_RGB8(&channels, 255, 255, 255);
    
    // Set transparency level
    set_transparency_level(overlay, transparency, &channels);
    
    // Apply base configuration
    ncplane_set_base(overlay, " ", 0, channels);
    
    return overlay;
}

int text_output_print_overlay(struct ncplane* overlay, int y, int x, 
                             output_color_t fg_color, text_style_t style,
                             const char* format, ...) {
    if (!overlay || !format) {
        return -1;
    }
    
    // Set up style
    ncplane_set_styles(overlay, style);
    
    // Set only foreground color, preserving background alpha settings
    uint64_t channels = ncplane_channels(overlay);
    uint8_t r, g, b;
    
    // Get RGB values for the foreground color
    switch (fg_color) {
        case COLOR_DEFAULT:
        case COLOR_WHITE:
            r = g = b = 255;
            break;
        case COLOR_BLACK:
            r = g = b = 0;
            break;
        case COLOR_RED:
            r = 255; g = 0; b = 0;
            break;
        case COLOR_GREEN:
            r = 0; g = 255; b = 0;
            break;
        case COLOR_YELLOW:
            r = 255; g = 255; b = 0;
            break;
        case COLOR_BLUE:
            r = 0; g = 0; b = 255;
            break;
        case COLOR_MAGENTA:
            r = 255; g = 0; b = 255;
            break;
        case COLOR_CYAN:
            r = 0; g = 255; b = 255;
            break;
        default:
            r = g = b = 255;
            break;
    }
    
    // Update only the foreground color
    NCCHANNELS_SET_FG_RGB8(&channels, r, g, b);
    ncplane_set_channels(overlay, channels);
    
    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Print the text on the overlay
    return ncplane_putstr_yx(overlay, y, x, buffer);
}

struct ncplane* text_output_display_shadow_text(int y, int x, output_color_t fg_color,
                                              text_style_t style, const char* format, ...) {
    if (!io_handler_ref || !io_handler_ref->nc || !format) {
        return NULL;
    }
    
    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Calculate required size (approximate)
    int len = strlen(buffer);
    int width = len + 4;  // Add some padding
    int height = 3;       // Shadow text is a single line with padding
    
    // Create an overlay with medium transparency for the shadow effect
    struct ncplane* overlay = text_output_create_overlay_sized(y, x, height, width, 
                                                            TEXT_TRANSPARENCY_BACKGROUND_HEAVY);
    if (!overlay) {
        return NULL;
    }
    
    // Draw shadow first (offset by 1,1)
    text_output_print_overlay(overlay, 1, 2, COLOR_BLACK, style, "%s", buffer);
    
    // Draw main text
    text_output_print_overlay(overlay, 0, 1, fg_color, style, "%s", buffer);
    
    
    return overlay;
}

struct ncplane* text_output_display_caption(int y, output_color_t fg_color,
                                          text_style_t style, const char* format, ...) {
    if (!io_handler_ref || !io_handler_ref->nc || !format) {
        return NULL;
    }
    
    // Format the string
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Get terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Calculate vertical position (negative y means from bottom)
    int ypos = (y < 0) ? rows + y : y;
    
    // Create an overlay for the caption with light transparency
    struct ncplane* overlay = text_output_create_overlay_sized(ypos, 0, 2, cols, 
                                                            TEXT_TRANSPARENCY_BACKGROUND_LIGHT);
    if (!overlay) {
        return NULL;
    }
    
    // Print the caption centered
    int ret = ncplane_putstr_aligned(overlay, 0, NCALIGN_CENTER, buffer);
    
    // Apply style and color
    uint64_t channels = ncplane_channels(overlay);
    uint8_t r, g, b;
    
    // Get RGB values for the foreground color
    switch (fg_color) {
        case COLOR_DEFAULT:
        case COLOR_WHITE:
            r = g = b = 255;
            break;
        case COLOR_BLACK:
            r = g = b = 0;
            break;
        case COLOR_RED:
            r = 255; g = 0; b = 0;
            break;
        case COLOR_GREEN:
            r = 0; g = 255; b = 0;
            break;
        case COLOR_YELLOW:
            r = 255; g = 255; b = 0;
            break;
        case COLOR_BLUE:
            r = 0; g = 0; b = 255;
            break;
        case COLOR_MAGENTA:
            r = 255; g = 0; b = 255;
            break;
        case COLOR_CYAN:
            r = 0; g = 255; b = 255;
            break;
        default:
            r = g = b = 255;
            break;
    }
    
    NCCHANNELS_SET_FG_RGB8(&channels, r, g, b);
    ncplane_set_channels(overlay, channels);
    ncplane_set_styles(overlay, style);
    
    
    return overlay;
}