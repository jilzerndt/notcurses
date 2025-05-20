#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "media_output_handler.h"
#include "output_handler.h"
#include "text_output_handler.h"
#include "../io_handler.h"

// External reference to the IO handler
extern io_handler_t* io_handler_ref;

/**
 * Display an image at specified coordinates and dimensions
 * @param filename Path to the image file
 * @param x X-coordinate (column) for the image
 * @param y Y-coordinate (row) for the image
 * @param width Width in columns (negative for auto-sizing)
 * @param height Height in rows (negative for auto-sizing)
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_image(const char *filename, int x, int y, int width, int height, media_scale_t scale) {
    if (!filename || !io_handler_ref) {
        return false;
    }
    
    // Check if images are supported
    if (!media_output_can_display_images()) {
        fprintf(stderr, "Image display not supported on this terminal\n");
        return false;
    }
    
    // Convert relative path to absolute if needed
    char full_path[1024];
    if (filename[0] != '/' && filename[0] != '\\' && io_handler_ref->base_path) {
        if (!io_handler_get_resource_path(io_handler_ref, filename, full_path, sizeof(full_path))) {
            strcpy(full_path, filename); // Fall back to original path
        }
    } else {
        strcpy(full_path, filename);
    }
    
    // Use the existing media_output_load_image function
    media_instance_t* media = media_output_load_image(full_path, y, x, height, width, scale, MEDIA_BLITTER_DEFAULT);
    if (!media) {
        fprintf(stderr, "Failed to load image: %s\n", full_path);
        return false;
    }
    
    // Render the image
    bool result = media_output_render(media);
    
    // Render to show the image
    io_handler_render(io_handler_ref);
    
    // Note: we don't destroy the media instance because we want the image to remain visible
    // The user should call media_output_destroy when they're done with the image
    
    return result;
}

/**
 * Display an image in fullscreen mode
 * @param filename Path to the image file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_image_fullscreen(const char *filename, media_scale_t scale) {
    if (!filename || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    
    // Get the terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Display the image at full terminal size (0,0 origin)
    return display_image(filename, 0, 0, cols, rows, scale);
}

/**
 * Display an image in a single cell (useful for map tiles)
 * @param filename Path to the image file
 * @param x X-coordinate (column) for the cell
 * @param y Y-coordinate (row) for the cell
 * @return true on success, false on failure
 */
bool display_image_cell(const char *filename, int x, int y) {
    if (!filename || !io_handler_ref) {
        return false;
    }
    
    // Check if images are supported
    if (!media_output_can_display_images()) {
        fprintf(stderr, "Image display not supported on this terminal\n");
        return false;
    }
    
    // Convert relative path to absolute if needed
    char full_path[1024];
    if (filename[0] != '/' && filename[0] != '\\' && io_handler_ref->base_path) {
        if (!io_handler_get_resource_path(io_handler_ref, filename, full_path, sizeof(full_path))) {
            strcpy(full_path, filename); // Fall back to original path
        }
    } else {
        strcpy(full_path, filename);
    }
    
    // Use the existing media_output_load_image function with special settings for cell display
    media_instance_t* media = media_output_load_image(full_path, y, x, 1, 1, MEDIA_SCALE_NONE, MEDIA_BLITTER_ASCII);
    if (!media) {
        fprintf(stderr, "Failed to load image: %s\n", full_path);
        return false;
    }
    
    // Render the image
    bool result = media_output_render(media);
    
    // Render to show the cell
    io_handler_render(io_handler_ref);
    
    // Note: we don't destroy the media instance because we want the image to remain visible
    // The user should call media_output_destroy when they're done with the image
    
    return result;
}

/**
 * Display a GIF animation at specified coordinates and dimensions
 * @param filename Path to the GIF file
 * @param x X-coordinate (column) for the animation
 * @param y Y-coordinate (row) for the animation
 * @param width Width in columns (negative for auto-sizing)
 * @param height Height in rows (negative for auto-sizing)
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_gif(const char *filename, int x, int y, int width, int height, media_scale_t scale) {
    if (!filename || !io_handler_ref) {
        return false;
    }
    
    // Check if images are supported
    if (!media_output_can_display_images()) {
        fprintf(stderr, "GIF animation not supported on this terminal\n");
        return false;
    }
    
    // Convert relative path to absolute if needed
    char full_path[1024];
    if (filename[0] != '/' && filename[0] != '\\' && io_handler_ref->base_path) {
        if (!io_handler_get_resource_path(io_handler_ref, filename, full_path, sizeof(full_path))) {
            strcpy(full_path, filename); // Fall back to original path
        }
    } else {
        strcpy(full_path, filename);
    }
    
    // Use the existing media_output_load_animation function
    media_instance_t* media = media_output_load_animation(full_path, y, x, height, width, scale, MEDIA_BLITTER_DEFAULT);
    if (!media) {
        fprintf(stderr, "Failed to load GIF: %s\n", full_path);
        return false;
    }
    
    // Start playing the animation
    media_output_play(media);
    
    // Render the first frame
    bool result = media_output_render(media);
    
    // Render to show the animation
    io_handler_render(io_handler_ref);
    
    // Note: Animation playback needs to be advanced in the main application loop
    // by calling media_output_render_next_frame regularly
    
    return result;
}

/**
 * Display a GIF animation in fullscreen mode
 * @param filename Path to the GIF file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_gif_fullscreen(const char *filename, media_scale_t scale) {
    if (!filename || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    
    // Get the terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Display the GIF at full terminal size (0,0 origin)
    return display_gif(filename, 0, 0, cols, rows, scale);
}

/**
 * Display a video at specified coordinates and dimensions
 * @param filename Path to the video file
 * @param x X-coordinate (column) for the video
 * @param y Y-coordinate (row) for the video
 * @param width Width in columns (negative for auto-sizing)
 * @param height Height in rows (negative for auto-sizing)
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_video(const char *filename, int x, int y, int width, int height, media_scale_t scale) {
    if (!filename || !io_handler_ref) {
        return false;
    }
    
    // Check if videos are supported
    if (!media_output_can_display_videos()) {
        fprintf(stderr, "Video playback not supported on this terminal\n");
        return false;
    }
    
    // Convert relative path to absolute if needed
    char full_path[1024];
    if (filename[0] != '/' && filename[0] != '\\' && io_handler_ref->base_path) {
        if (!io_handler_get_resource_path(io_handler_ref, filename, full_path, sizeof(full_path))) {
            strcpy(full_path, filename); // Fall back to original path
        }
    } else {
        strcpy(full_path, filename);
    }
    
    // Use the existing media_output_load_video function
    media_instance_t* media = media_output_load_video(full_path, y, x, height, width, scale, MEDIA_BLITTER_DEFAULT);
    if (!media) {
        fprintf(stderr, "Failed to load video: %s\n", full_path);
        return false;
    }
    
    // Start playing the video
    media_output_play(media);
    
    // Render the first frame
    bool result = media_output_render(media);
    
    // Render to show the video
    io_handler_render(io_handler_ref);
    
    // Note: Video playback needs to be advanced in the main application loop
    // by calling media_output_render_next_frame regularly
    
    return result;
}

/**
 * Display a video in fullscreen mode
 * @param filename Path to the video file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_video_fullscreen(const char *filename, media_scale_t scale) {
    if (!filename || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    
    // Get the terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Display the video at full terminal size (0,0 origin)
    return display_video(filename, 0, 0, cols, rows, scale);
}

/**
 * Create an overlay plane for text over fullscreen media
 * 
 * HOW TO DISPLAY TEXT OVER FULLSCREEN MEDIA:
 * 
 * When you display media in fullscreen mode (using display_image_fullscreen,
 * display_gif_fullscreen, or display_video_fullscreen), you may want to overlay
 * text on top of it. Here's how to do it:
 * 
 * 1. Create a transparent overlay plane:
 *    struct ncplane* overlay = create_text_overlay();
 * 
 * 2. Use the text_output functions to draw on the overlay:
 *    text_output_print(overlay, y, x, COLOR_WHITE, COLOR_BLACK, TEXT_STYLE_BOLD, "Your text here");
 * 
 * 3. You can also use fancy_output to draw UI elements:
 *    fancy_output_draw_box(overlay, y, x, height, width, BOX_STYLE_SINGLE, COLOR_WHITE, COLOR_BLACK);
 * 
 * 4. When all overlay content is set, render everything:
 *    io_handler_render(io_handler_ref);
 * 
 * This technique creates a transparent plane on top of your media where
 * only the text is visible, allowing the media to show through around the text.
 * For UI elements like boxes or panels, you can use partial transparency by
 * setting the background alpha to a value between NCALPHA_TRANSPARENT and
 * NCALPHA_OPAQUE.
 */
struct ncplane* create_text_overlay(void) {
    if (!io_handler_ref || !io_handler_ref->nc || !io_handler_ref->stdplane) {
        return NULL;
    }
    
    // Get the terminal dimensions
    unsigned rows, cols;
    notcurses_term_dim_yx(io_handler_ref->nc, &rows, &cols);
    
    // Create a transparent overlay plane for text
    struct ncplane* overlay = ncplane_create(io_handler_ref->stdplane, 
                                           &(struct ncplane_options){
                                              .y = 0,
                                              .x = 0,
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
    NCCHANNELS_SET_FG_RGB8(&channels, 255, 255, 255); // White text
    NCCHANNELS_SET_FG_ALPHA(&channels, NCALPHA_OPAQUE);
    NCCHANNELS_SET_BG_ALPHA(&channels, NCALPHA_TRANSPARENT); // Transparent background
    ncplane_set_base(overlay, " ", 0, channels);
    
    return overlay;
}