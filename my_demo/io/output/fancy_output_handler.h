#ifndef FANCY_OUTPUT_HANDLER_H
#define FANCY_OUTPUT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <notcurses/notcurses.h>
#include "output_handler.h"

/**
 * Box drawing styles
 */
typedef enum box_style {
    BOX_STYLE_SINGLE,    // Single-line box drawing characters
    BOX_STYLE_DOUBLE,    // Double-line box drawing characters
    BOX_STYLE_ROUNDED,   // Rounded corners
    BOX_STYLE_HEAVY,     // Heavy borders
    BOX_STYLE_ASCII      // Simple ASCII characters
} box_style_t;

/**
 * Menu item structure
 */
typedef struct menu_item {
    char* text;           // Text to display
    int id;               // Unique identifier for the item
    bool enabled;         // Whether the item can be selected
} menu_item_t;

/**
 * Menu callback function type
 * @param id Selected menu item ID
 * @param user_data User-provided data pointer
 */
typedef void (*menu_callback_t)(int id, void* user_data);

/**
 * Menu structure for interactive selection
 */
typedef struct menu {
    struct ncplane* plane;      // Plane where the menu is drawn
    struct ncselector* selector; // Notcurses selector widget
    menu_item_t* items;         // Array of menu items
    int item_count;             // Number of items
    int selected_index;         // Currently selected item index
    menu_callback_t callback;   // Callback for selection
    void* user_data;            // User data for callback
} menu_t;

/**
 * Draw a simple box with specified style
 * @param plane Target plane
 * @param y Y-coordinate (row) for top-left corner
 * @param x X-coordinate (column) for top-left corner
 * @param height Box height
 * @param width Box width
 * @param style Box drawing style
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @return true on success, false on failure
 */
bool fancy_output_draw_box(struct ncplane* plane, int y, int x, 
                          int height, int width, box_style_t style,
                          output_color_t fg_color, output_color_t bg_color);

/**
 * Draw a titled box with specified style
 * @param plane Target plane
 * @param y Y-coordinate (row) for top-left corner
 * @param x X-coordinate (column) for top-left corner
 * @param height Box height
 * @param width Box width
 * @param style Box drawing style
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param title Box title text
 * @return true on success, false on failure
 */
bool fancy_output_draw_titled_box(struct ncplane* plane, int y, int x, 
                                 int height, int width, box_style_t style,
                                 output_color_t fg_color, output_color_t bg_color,
                                 const char* title);

/**
 * Draw a horizontal bar (progress bar)
 * @param plane Target plane
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column)
 * @param width Total width of the bar
 * @param percentage Fill percentage (0-100)
 * @param empty_color Color for empty portion
 * @param fill_color Color for filled portion
 * @return true on success, false on failure
 */
bool fancy_output_draw_progress_bar(struct ncplane* plane, int y, int x, 
                                   int width, int percentage,
                                   output_color_t empty_color, 
                                   output_color_t fill_color);

/**
 * Create a menu on a new plane
 * @param y Y-coordinate (row) for menu
 * @param x X-coordinate (column) for menu
 * @param items Array of menu items
 * @param item_count Number of items in array
 * @param title Menu title
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param callback Function to call when item is selected
 * @param user_data Data to pass to callback
 * @return Menu structure or NULL on failure
 */
menu_t* fancy_output_create_menu(int y, int x, menu_item_t* items, int item_count,
                               const char* title, output_color_t fg_color, 
                               output_color_t bg_color, menu_callback_t callback,
                               void* user_data);

/**
 * Process input for a menu
 * @param menu Menu to process input for
 * @param key Input key
 * @return true if menu handled the key, false otherwise
 */
bool fancy_output_menu_input(menu_t* menu, uint32_t key);

/**
 * Destroy a menu and free all resources
 * @param menu Menu to destroy
 */
void fancy_output_destroy_menu(menu_t* menu);

/**
 * Draw a horizontal line with specified style
 * @param plane Target plane
 * @param y Y-coordinate (row)
 * @param x X-coordinate (column) for start
 * @param length Line length
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @return true on success, false on failure
 */
bool fancy_output_draw_hline(struct ncplane* plane, int y, int x, 
                            int length, output_color_t fg_color, 
                            output_color_t bg_color);

/**
 * Draw a vertical line with specified style
 * @param plane Target plane
 * @param y Y-coordinate (row) for start
 * @param x X-coordinate (column)
 * @param length Line length
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @return true on success, false on failure
 */
bool fancy_output_draw_vline(struct ncplane* plane, int y, int x, 
                            int length, output_color_t fg_color, 
                            output_color_t bg_color);

/**
 * Draw a simple popup message with OK button
 * @param title Popup title
 * @param message Message text
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @return true on success, false on failure
 */
bool fancy_output_show_popup(const char* title, const char* message,
                            output_color_t fg_color, output_color_t bg_color);

#endif /* FANCY_OUTPUT_HANDLER_H */