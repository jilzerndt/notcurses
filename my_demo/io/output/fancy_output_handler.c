#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fancy_output_handler.h"
#include "text_output_handler.h"
#include "../io_handler.h"

// Reference to the IO handler
static io_handler_t* io_handler_ref = NULL;

// Helper function to get box characters based on style
static const wchar_t* get_box_chars(box_style_t style) {
    switch (style) {
        case BOX_STYLE_SINGLE:
            return NCBOXLIGHT;  // Single-line box
        case BOX_STYLE_DOUBLE:
            return NCBOXDOUBLE; // Double-line box
        case BOX_STYLE_ROUNDED:
            return NCBOXROUNDED; // Rounded corners
        case BOX_STYLE_HEAVY:
            return NCBOXHEAVY;  // Heavy/thick lines
        case BOX_STYLE_ASCII:
            return NCBOXASCII;  // ASCII-only box
        default:
            return NCBOXLIGHT;  // Default to light box
    }
}

// Initialize the fancy output handler
void fancy_output_init(io_handler_t* io_handler) {
    io_handler_ref = io_handler;
}

bool fancy_output_draw_box(struct ncplane* plane, int y, int x, 
                          int height, int width, box_style_t style,
                          output_color_t fg_color, output_color_t bg_color) {
    if (!plane || height < 2 || width < 2) {
        return false;
    }
    
    // Set colors
    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    
    // Get box characters based on style
    const wchar_t* box_chars = get_box_chars(style);
    
    // Draw the box
    return ncplane_box_sized(plane, y, x, height, width, 0, channels, box_chars) == 0;
}

bool fancy_output_draw_titled_box(struct ncplane* plane, int y, int x, 
                                 int height, int width, box_style_t style,
                                 output_color_t fg_color, output_color_t bg_color,
                                 const char* title) {
    if (!plane || height < 2 || width < 2 || !title) {
        return false;
    }
    
    // Draw the box first
    if (!fancy_output_draw_box(plane, y, x, height, width, style, fg_color, bg_color)) {
        return false;
    }
    
    // Calculate title position (centered on top line)
    int title_len = strlen(title);
    if (title_len > width - 4) {
        title_len = width - 4; // Truncate if too long
    }
    
    int title_x = x + (width - title_len) / 2;
    
    // Draw the title
    return text_output_print(plane, y, title_x, fg_color, bg_color, 
                            TEXT_STYLE_BOLD, " %.*s ", title_len, title) > 0;
}

bool fancy_output_draw_progress_bar(struct ncplane* plane, int y, int x, 
                                   int width, int percentage,
                                   output_color_t empty_color, 
                                   output_color_t fill_color) {
    if (!plane || width < 3 || percentage < 0 || percentage > 100) {
        return false;
    }
    
    // Ensure valid percentage
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    // Calculate filled width
    int filled_width = (width * percentage) / 100;
    if (filled_width > width) filled_width = width;
    
    // Get colors
    uint64_t empty_channels = 0;
    uint64_t fill_channels = 0;
    output_handler_set_fg_color(empty_color, &empty_channels);
    output_handler_set_bg_color(COLOR_BLACK, &empty_channels);
    output_handler_set_fg_color(fill_color, &fill_channels);
    output_handler_set_bg_color(COLOR_BLACK, &fill_channels);
    
    // Draw filled part
    if (filled_width > 0) {
        ncplane_set_channels(plane, fill_channels);
        for (int i = 0; i < filled_width; i++) {
            ncplane_putchar_yx(plane, y, x + i, '█');
        }
    }
    
    // Draw empty part
    if (filled_width < width) {
        ncplane_set_channels(plane, empty_channels);
        for (int i = filled_width; i < width; i++) {
            ncplane_putchar_yx(plane, y, x + i, '░');
        }
    }
    
    return true;
}

menu_t* fancy_output_create_menu(int y, int x, menu_item_t* items, int item_count,
                                const char* title, output_color_t fg_color, 
                                output_color_t bg_color, menu_callback_t callback,
                                void* user_data) {
    if (!items || item_count <= 0 || !title || !io_handler_ref || !io_handler_ref->nc) {
        return NULL;
    }
    
    // Create new menu struct
    menu_t* menu = malloc(sizeof(menu_t));
    if (!menu) {
        return NULL;
    }
    
    // Initialize menu struct
    menu->items = malloc(item_count * sizeof(menu_item_t));
    if (!menu->items) {
        free(menu);
        return NULL;
    }
    
    // Copy items
    for (int i = 0; i < item_count; i++) {
        menu->items[i].text = strdup(items[i].text);
        menu->items[i].id = items[i].id;
        menu->items[i].enabled = items[i].enabled;
    }
    
    menu->item_count = item_count;
    menu->selected_index = 0;
    menu->callback = callback;
    menu->user_data = user_data;
    
    // Calculate menu size (based on longest item + padding)
    int max_len = strlen(title);
    for (int i = 0; i < item_count; i++) {
        int len = strlen(items[i].text);
        if (len > max_len) {
            max_len = len;
        }
    }
    
    int menu_width = max_len + 6; // Add padding
    int menu_height = item_count + 4; // Title, items, padding
    
    // Create plane for the menu
    menu->plane = ncplane_create(io_handler_ref->stdplane, &(struct ncplane_options){
        .y = y,
        .x = x,
        .rows = menu_height,
        .cols = menu_width,
        .name = "menu",
        .resizecb = NULL,
        .flags = 0,
    });
    
    if (!menu->plane) {
        for (int i = 0; i < item_count; i++) {
            free(menu->items[i].text);
        }
        free(menu->items);
        free(menu);
        return NULL;
    }
    
    // Set up colors
    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    ncplane_set_base(menu->plane, " ", 0, channels);
    
    // Draw menu border with title
    fancy_output_draw_titled_box(menu->plane, 0, 0, menu_height, menu_width, 
                               BOX_STYLE_SINGLE, fg_color, bg_color, title);
    
    // Create selector options
    struct ncselector_item* selector_items = malloc(item_count * sizeof(struct ncselector_item));
    if (!selector_items) {
        ncplane_destroy(menu->plane);
        for (int i = 0; i < item_count; i++) {
            free(menu->items[i].text);
        }
        free(menu->items);
        free(menu);
        return NULL;
    }
    
    // Copy items to selector format
    for (int i = 0; i < item_count; i++) {
        selector_items[i].option = menu->items[i].text;
        selector_items[i].desc = "";  // No description for now
    }
    
    // Create notcurses selector
    menu->selector = ncselector_create(menu->plane, 1, 1, menu->item_count);
    if (!menu->selector) {
        free(selector_items);
        ncplane_destroy(menu->plane);
        for (int i = 0; i < item_count; i++) {
            free(menu->items[i].text);
        }
        free(menu->items);
        free(menu);
        return NULL;
    }
    
    // Set selector items
    ncselector_set_secondary(menu->selector, channels);
    ncselector_set_options(menu->selector, selector_items, item_count);
    
    free(selector_items);
    return menu;
}

bool fancy_output_menu_input(menu_t* menu, uint32_t key) {
    if (!menu || !menu->selector) {
        return false;
    }
    
    // Process key
    switch (key) {
        case NCKEY_UP:
        case 'k':
            ncselector_previtem(menu->selector);
            return true;
            
        case NCKEY_DOWN:
        case 'j':
            ncselector_nextitem(menu->selector);
            return true;
            
        case NCKEY_ENTER:
        case ' ':
            // Get selected option
            menu->selected_index = ncselector_selected(menu->selector);
            
            // Call the callback if provided
            if (menu->callback && menu->selected_index >= 0 && 
                menu->selected_index < menu->item_count) {
                menu->callback(menu->items[menu->selected_index].id, menu->user_data);
            }
            return true;
            
        default:
            return false;
    }
}

void fancy_output_destroy_menu(menu_t* menu) {
    if (!menu) {
        return;
    }
    
    // Destroy selector
    if (menu->selector) {
        ncselector_destroy(menu->selector, NULL);
    }
    
    // Destroy plane
    if (menu->plane) {
        ncplane_destroy(menu->plane);
    }
    
    // Free items
    if (menu->items) {
        for (int i = 0; i < menu->item_count; i++) {
            free(menu->items[i].text);
        }
        free(menu->items);
    }
    
    free(menu);
}

bool fancy_output_draw_hline(struct ncplane* plane, int y, int x, 
                            int length, output_color_t fg_color, 
                            output_color_t bg_color) {
    if (!plane || length < 1) {
        return false;
    }
    
    // Set colors
    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    
    // Draw horizontal line
    return ncplane_hline_interp(plane, "─", length, channels, channels) >= 0;
}

bool fancy_output_draw_vline(struct ncplane* plane, int y, int x, 
                            int length, output_color_t fg_color, 
                            output_color_t bg_color) {
    if (!plane || length < 1) {
        return false;
    }
    
    // Set colors
    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    
    // Draw vertical line
    return ncplane_vline_interp(plane, "│", length, channels, channels) >= 0;
}

bool fancy_output_show_popup(const char* title, const char* message,
                            output_color_t fg_color, output_color_t bg_color) {
    if (!title || !message || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    
    // Calculate popup size based on message
    int msg_len = strlen(message);
    int rows = 2; // Minimum 2 rows for the message
    int cols = msg_len + 4; // Padding on both sides
    
    // Add rows for line breaks
    for (int i = 0; i < msg_len; i++) {
        if (message[i] == '\n') {
            rows++;
        }
    }
    
    // Set minimum size
    if (cols < 20) cols = 20;
    if (rows < 5) rows = 5;
    
    // Add rows for title and OK button
    rows += 4;
    
    // Get screen size to center the popup
    unsigned term_rows, term_cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &term_rows, &term_cols);
    
    int y = (term_rows - rows) / 2;
    int x = (term_cols - cols) / 2;
    
    // Create popup plane
    struct ncplane* popup = ncplane_create(io_handler_ref->stdplane, &(struct ncplane_options){
        .y = y,
        .x = x,
        .rows = rows,
        .cols = cols,
        .name = "popup",
        .resizecb = NULL,
        .flags = 0,
    });
    
    if (!popup) {
        return false;
    }
    
    // Set background
    uint64_t channels = 0;
    output_handler_set_fg_color(fg_color, &channels);
    output_handler_set_bg_color(bg_color, &channels);
    ncplane_set_base(popup, " ", 0, channels);
    
    // Draw box with title
    fancy_output_draw_titled_box(popup, 0, 0, rows, cols, BOX_STYLE_DOUBLE, 
                               fg_color, bg_color, title);
    
    // Draw message
    text_output_print(popup, 2, 2, fg_color, bg_color, TEXT_STYLE_NORMAL, "%s", message);
    
    // Draw OK button
    int button_y = rows - 2;
    int button_x = (cols - 6) / 2;
    fancy_output_draw_box(popup, button_y, button_x, 3, 6, BOX_STYLE_SINGLE, 
                        fg_color, bg_color);
    text_output_print(popup, button_y + 1, button_x + 1, fg_color, bg_color, 
                     TEXT_STYLE_BOLD, "OK");
    
    // Render the popup
    notcurses_render(io_handler_ref->nc);
    
    // Wait for any key
    notcurses_get_blocking(io_handler_ref->nc, NULL);
    
    // Destroy popup
    ncplane_destroy(popup);
    
    return true;
}