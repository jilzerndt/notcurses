#ifndef MEDIA_OUTPUT_HANDLER_H
#define MEDIA_OUTPUT_HANDLER_H

#include <stdbool.h>
#include <notcurses/notcurses.h>
#include "output_handler.h"

/**
 * Media scaling options
 */
typedef enum media_scale {
    MEDIA_SCALE_NONE = NCSCALE_NONE,         // No scaling
    MEDIA_SCALE_SCALE = NCSCALE_SCALE,       // Scale, preserving aspect ratio
    MEDIA_SCALE_STRETCH = NCSCALE_STRETCH,   // Stretch to fit, ignoring aspect ratio
    MEDIA_SCALE_FIT = NCSCALE_SCALE_HIRES,   // Scale with focus on high resolution
} media_scale_t;

/**
 * Media blitter options (rendering method)
 */
typedef enum media_blitter {
    MEDIA_BLITTER_DEFAULT = NCBLIT_DEFAULT,  // Use terminal's best option
    MEDIA_BLITTER_ASCII = NCBLIT_1x1,        // Use ASCII only
    MEDIA_BLITTER_HALF = NCBLIT_2x1,         // Use half blocks (▀ ▄)
    MEDIA_BLITTER_QUAD = NCBLIT_2x2,         // Use quadrant blocks (▖ ▗ ▘ ▙)
    MEDIA_BLITTER_BRAILLE = NCBLIT_BRAILLE,  // Use braille (⠀⠁⠂⠃...)
    MEDIA_BLITTER_PIXEL = NCBLIT_PIXEL,      // Use pixel graphics if available
} media_blitter_t;

/**
 * Media instance structure to track loaded visual elements
 * Allows for multiple media objects to exist simultaneously
 */
typedef struct media_instance {
    struct ncvisual* visual;       // Notcurses visual object
    struct ncplane* plane;         // Plane where this media is displayed
    int frame_count;               // For animations, total frame count
    int current_frame;             // For animations, current frame
    bool is_playing;               // For animations, playing state
} media_instance_t;

/**
 * Load an image file (.png, .jpg, etc.) and render it to a plane
 * @param file_path Path to the image file
 * @param y Y-coordinate (row) for media plane
 * @param x X-coordinate (column) for media plane
 * @param rows Maximum rows (height) for media, or -1 for auto
 * @param cols Maximum columns (width) for media, or -1 for auto
 * @param scaling Media scaling method
 * @param blitter Media blitter (rendering method)
 * @return Pointer to media instance or NULL on failure
 */
media_instance_t* media_output_load_image(const char* file_path, int y, int x, 
                                     int rows, int cols, 
                                     media_scale_t scaling, 
                                     media_blitter_t blitter);

/**
 * Load an animation file (.gif) and prepare it for playback
 * @param file_path Path to the GIF file
 * @param y Y-coordinate (row) for media plane
 * @param x X-coordinate (column) for media plane
 * @param rows Maximum rows (height) for media, or -1 for auto
 * @param cols Maximum columns (width) for media, or -1 for auto
 * @param scaling Media scaling method
 * @param blitter Media blitter (rendering method)
 * @return Pointer to media instance or NULL on failure
 */
media_instance_t* media_output_load_animation(const char* file_path, int y, int x, 
                                        int rows, int cols, 
                                        media_scale_t scaling, 
                                        media_blitter_t blitter);

/**
 * Load a video file (.mp4) and prepare it for playback
 * @param file_path Path to the video file
 * @param y Y-coordinate (row) for media plane
 * @param x X-coordinate (column) for media plane
 * @param rows Maximum rows (height) for media, or -1 for auto
 * @param cols Maximum columns (width) for media, or -1 for auto
 * @param scaling Media scaling method
 * @param blitter Media blitter (rendering method)
 * @return Pointer to media instance or NULL on failure
 */
media_instance_t* media_output_load_video(const char* file_path, int y, int x, 
                                    int rows, int cols, 
                                    media_scale_t scaling, 
                                    media_blitter_t blitter);

/**
 * Display a loaded media file on its assigned plane
 * @param media Pointer to a loaded media instance
 * @return true on success, false on failure
 */
bool media_output_render(media_instance_t* media);

/**
 * Display the next frame of an animation or video
 * @param media Pointer to a loaded media instance
 * @return true on success, false on failure or end of media
 */
bool media_output_render_next_frame(media_instance_t* media);

/**
 * Start playback of an animation or video
 * Note: This does not handle actual timing, which must be done in the game loop
 * @param media Pointer to a loaded media instance
 */
void media_output_play(media_instance_t* media);

/**
 * Pause playback of an animation or video
 * @param media Pointer to a loaded media instance
 */
void media_output_pause(media_instance_t* media);

/**
 * Reset animation or video to the beginning
 * @param media Pointer to a loaded media instance
 */
void media_output_reset(media_instance_t* media);

/**
 * Move a media plane to a new position
 * @param media Pointer to a loaded media instance
 * @param y New Y-coordinate (row)
 * @param x New X-coordinate (column)
 * @return true on success, false on failure
 */
bool media_output_move(media_instance_t* media, int y, int x);

/**
 * Free all resources associated with a media instance
 * @param media Pointer to a loaded media instance
 */
void media_output_destroy(media_instance_t* media);

/**
 * Check if the notcurses implementation supports image loading
 * @return true if supported, false otherwise
 */
bool media_output_can_display_images(void);

/**
 * Check if the notcurses implementation supports video loading
 * @return true if supported, false otherwise
 */
bool media_output_can_display_videos(void);

#endif /* MEDIA_OUTPUT_HANDLER_H */