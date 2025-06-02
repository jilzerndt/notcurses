#ifndef TEXT_OUTPUT_HANDLER_H
#define TEXT_OUTPUT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <notcurses/notcurses.h>
#include "output_handler.h"

/*
 * RENDERING APPROACH:
 * This text output handler does NOT perform automatic rendering.
 * All functions modify plane content only. To display changes:
 * 1. Call text output functions to modify plane content
 * 2. Call output_handler_render() when ready to display
 * 
 * This allows efficient batching of multiple text operations
 * and leverages notcurses' automatic damage detection.
 */

/**
 * Text alignment options
 */
typedef enum text_align {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
} text_align_t;

/**
 * Text style flags
 */
typedef enum text_style {
    TEXT_STYLE_NORMAL   = 0,
    TEXT_STYLE_BOLD     = NCSTYLE_BOLD,
    TEXT_STYLE_ITALIC   = NCSTYLE_ITALIC,
    TEXT_STYLE_UNDERLINE = NCSTYLE_UNDERLINE
} text_style_t;

/**
 * Transparency level for overlay planes
 */
typedef enum text_transparency {
    TEXT_TRANSPARENCY_NONE = 0,         // Fully opaque
    TEXT_TRANSPARENCY_TEXT_ONLY,        // Only text visible, background transparent
    TEXT_TRANSPARENCY_BACKGROUND_LIGHT, // Background slightly visible (25%)
    TEXT_TRANSPARENCY_BACKGROUND_MEDIUM,// Background partially visible (50%)
    TEXT_TRANSPARENCY_BACKGROUND_HEAVY  // Background mostly visible (75%)
} text_transparency_t;

/**
 * Print text on the specified plane at given coordinates
 * @param plane Target plane
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print(struct ncplane* plane, int y, int x, 
                      output_color_t fg_color, output_color_t bg_color, 
                      text_style_t style, const char* format, ...);

/**
 * Print text on the game plane at given coordinates
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print_game(int y, int x, output_color_t fg_color, 
                          output_color_t bg_color, text_style_t style, 
                          const char* format, ...);

/**
 * Print text on the UI plane at given coordinates
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print_ui(int y, int x, output_color_t fg_color, 
                         output_color_t bg_color, text_style_t style, 
                         const char* format, ...);

/**
 * Print a message to the message plane
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print_message(output_color_t fg_color, output_color_t bg_color, 
                             text_style_t style, const char* format, ...);

/**
 * Print aligned text on a plane
 * @param plane Target plane
 * @param y Y-coordinate (row)
 * @param align Text alignment
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print_aligned(struct ncplane* plane, int y, text_align_t align, 
                             output_color_t fg_color, output_color_t bg_color,
                             text_style_t style, const char* format, ...);

/**
 * Clear message plane and print a new message
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_clear_message(output_color_t fg_color, output_color_t bg_color, 
                             text_style_t style, const char* format, ...);

/**
 * Create a transparent overlay plane for displaying text over other content
 * @param transparency Level of transparency to use
 * @return New plane for overlay text
 */
struct ncplane* text_output_create_overlay(text_transparency_t transparency);

/**
 * Create a transparent overlay panel with a specified size and position
 * @param y Y-coordinate (row) for the top-left corner
 * @param x X-coordinate (column) for the top-left corner
 * @param rows Number of rows for the overlay
 * @param cols Number of columns for the overlay
 * @param transparency Level of transparency to use
 * @return New plane for overlay text
 */
struct ncplane* text_output_create_overlay_sized(int y, int x, int rows, int cols, 
                                               text_transparency_t transparency);

/**
 * Print text on a transparent overlay
 * @param overlay Target overlay plane
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param fg_color Foreground color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Characters printed, negative on error
 */
int text_output_print_overlay(struct ncplane* overlay, int y, int x, 
                             output_color_t fg_color, text_style_t style,
                             const char* format, ...);

/**
 * Create a text overlay and display a message with a shadow effect
 * (Useful for dialogs, notifications or captions)
 * Note: Call output_handler_render() after this to display the shadow text
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param fg_color Foreground color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Pointer to the overlay plane or NULL on failure
 */
struct ncplane* text_output_display_shadow_text(int y, int x, output_color_t fg_color,
                                              text_style_t style, const char* format, ...);

/**
 * Create and display a centered caption overlay over the full screen
 * (Useful for fullscreen images/videos)
 * Note: Call output_handler_render() after this to display the caption
 * @param y Y-coordinate (row), use negative value for bottom alignment
 * @param fg_color Foreground color
 * @param style Text style flags (can be combined with |)
 * @param format Printf-style format string
 * @param ... Variable arguments for format string
 * @return Pointer to the overlay plane or NULL on failure
 */
struct ncplane* text_output_display_caption(int y, output_color_t fg_color,
                                          text_style_t style, const char* format, ...);

#endif /* TEXT_OUTPUT_HANDLER_H */