#ifndef MEDIA_OUTPUT_SPECIFIC_H
#define MEDIA_OUTPUT_SPECIFIC_H

#include <stdbool.h>
#include <notcurses/notcurses.h>
#include "media_output_handler.h"

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
bool display_image(const char *filename, int x, int y, int width, int height, media_scale_t scale);

/**
 * Display an image in fullscreen mode
 * @param filename Path to the image file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_image_fullscreen(const char *filename, media_scale_t scale);

/**
 * Display an image in a single cell (useful for map tiles)
 * @param filename Path to the image file
 * @param x X-coordinate (column) for the cell
 * @param y Y-coordinate (row) for the cell
 * @return true on success, false on failure
 */
bool display_image_cell(const char *filename, int x, int y);

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
bool display_gif(const char *filename, int x, int y, int width, int height, media_scale_t scale);

/**
 * Display a GIF animation in fullscreen mode
 * @param filename Path to the GIF file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_gif_fullscreen(const char *filename, media_scale_t scale);

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
bool display_video(const char *filename, int x, int y, int width, int height, media_scale_t scale);

/**
 * Display a video in fullscreen mode
 * @param filename Path to the video file
 * @param scale Scaling method to use
 * @return true on success, false on failure
 */
bool display_video_fullscreen(const char *filename, media_scale_t scale);

/**
 * Create an overlay plane for text over fullscreen media
 * This creates a transparent plane on top of other content where text can be rendered
 * @return Pointer to the created plane or NULL on failure
 */
struct ncplane* create_text_overlay(void);

#endif /* MEDIA_OUTPUT_SPECIFIC_H */