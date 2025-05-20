#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "media_output_handler.h"
#include "../io_handler.h"

// Reference to the IO handler for capability checks
static io_handler_t* io_handler_ref = NULL;

// Helper function to create and set up a media instance
static media_instance_t* create_media_instance(struct ncvisual* visual, int y, int x,
                                              int rows, int cols,
                                              media_scale_t scaling,
                                              media_blitter_t blitter) {
    if (!visual || !io_handler_ref || !io_handler_ref->nc) {
        return NULL;
    }

    // Allocate memory for the media instance
    media_instance_t* media = malloc(sizeof(media_instance_t));
    if (!media) {
        ncvisual_destroy(visual);
        return NULL;
    }

    // Initialize with default values
    media->visual = visual;
    media->plane = NULL;
    media->frame_count = 1;
    media->current_frame = 0;
    media->is_playing = false;

    // Get media dimensions
    struct ncvisual_options vopts = {
        .n = NULL,
        .scaling = (ncscale_e)scaling,
        .blitter = (ncblitter_e)blitter,
        .flags = 0,
        .y = y,
        .x = x,
        .begy = 0,
        .begx = 0,
        .leny = rows < 0 ? 0 : rows,
        .lenx = cols < 0 ? 0 : cols
    };

    // Create a plane and render the visual
    media->plane = ncvisual_render(io_handler_ref->nc, media->visual, &vopts);
    
    if (!media->plane) {
        ncvisual_destroy(visual);
        free(media);
        return NULL;
    }

    return media;
}

// Helper function for common loading code
static media_instance_t* load_media_file(const char* file_path, int y, int x,
                                       int rows, int cols,
                                       media_scale_t scaling,
                                       media_blitter_t blitter) {
    if (!file_path || !io_handler_ref || !io_handler_ref->nc) {
        return NULL;
    }

    // Load the visual from file
    struct ncvisual* visual = ncvisual_from_file(file_path);
    if (!visual) {
        fprintf(stderr, "Failed to load media file: %s\n", file_path);
        return NULL;
    }

    return create_media_instance(visual, y, x, rows, cols, scaling, blitter);
}

// Initialize the media output handler
void media_output_init(io_handler_t* io_handler) {
    io_handler_ref = io_handler;
}

media_instance_t* media_output_load_image(const char* file_path, int y, int x,
                                        int rows, int cols,
                                        media_scale_t scaling,
                                        media_blitter_t blitter) {
    if (!media_output_can_display_images()) {
        fprintf(stderr, "Image display not supported\n");
        return NULL;
    }

    return load_media_file(file_path, y, x, rows, cols, scaling, blitter);
}

media_instance_t* media_output_load_animation(const char* file_path, int y, int x,
                                           int rows, int cols,
                                           media_scale_t scaling,
                                           media_blitter_t blitter) {
    if (!media_output_can_display_images()) {
        fprintf(stderr, "Animation display not supported\n");
        return NULL;
    }

    media_instance_t* media = load_media_file(file_path, y, x, rows, cols, scaling, blitter);
    if (!media) {
        return NULL;
    }

    // Count the number of frames in the animation
    int count = 0;
    struct ncvisual* temp = ncvisual_from_file(file_path);
    if (temp) {
        do {
            count++;
        } while (ncvisual_decode(temp) != NULL);
        ncvisual_destroy(temp);
    }

    media->frame_count = count > 0 ? count : 1;
    return media;
}

media_instance_t* media_output_load_video(const char* file_path, int y, int x,
                                       int rows, int cols,
                                       media_scale_t scaling,
                                       media_blitter_t blitter) {
    if (!media_output_can_display_videos()) {
        fprintf(stderr, "Video display not supported\n");
        return NULL;
    }

    media_instance_t* media = load_media_file(file_path, y, x, rows, cols, scaling, blitter);
    if (!media) {
        return NULL;
    }

    // Approximate the frame count based on video length
    // This is not accurate but gives some indication
    media->frame_count = 100;  // Default value
    return media;
}

bool media_output_render(media_instance_t* media) {
    if (!media || !media->visual || !media->plane || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }

    // Render the current frame
    struct ncvisual_options vopts = {
        .n = media->plane,
        .scaling = NCSCALE_SCALE,
        .blitter = NCBLIT_DEFAULT,
        .flags = 0,
        .y = -1,
        .x = -1,
        .begy = 0,
        .begx = 0,
        .leny = 0,
        .lenx = 0
    };

    return ncvisual_render(io_handler_ref->nc, media->visual, &vopts) != NULL;
}

bool media_output_render_next_frame(media_instance_t* media) {
    if (!media || !media->visual || !media->plane || !io_handler_ref || !io_handler_ref->nc) {
        return false;
    }

    // Decode the next frame
    if (ncvisual_decode(media->visual) == NULL) {
        // End of media reached, reset to beginning if playing
        if (media->is_playing) {
            media_output_reset(media);
            if (ncvisual_decode(media->visual) == NULL) {
                return false;  // Failed to reset
            }
        } else {
            return false;
        }
    }

    media->current_frame = (media->current_frame + 1) % media->frame_count;

    // Render the current frame
    return media_output_render(media);
}

void media_output_play(media_instance_t* media) {
    if (!media) {
        return;
    }
    media->is_playing = true;
}

void media_output_pause(media_instance_t* media) {
    if (!media) {
        return;
    }
    media->is_playing = false;
}

void media_output_reset(media_instance_t* media) {
    if (!media || !media->visual) {
        return;
    }

    // Reload the visual from the file
    const char* source = ncvisual_media(media->visual);
    if (source) {
        struct ncvisual* new_visual = ncvisual_from_file(source);
        if (new_visual) {
            ncvisual_destroy(media->visual);
            media->visual = new_visual;
            media->current_frame = 0;
        }
    }
}

bool media_output_move(media_instance_t* media, int y, int x) {
    if (!media || !media->plane) {
        return false;
    }

    return ncplane_move_yx(media->plane, y, x) == 0;
}

void media_output_destroy(media_instance_t* media) {
    if (!media) {
        return;
    }

    if (media->visual) {
        ncvisual_destroy(media->visual);
    }

    if (media->plane) {
        ncplane_destroy(media->plane);
    }

    free(media);
}

bool media_output_can_display_images(void) {
    if (!io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    return notcurses_canopen_images(io_handler_ref->nc);
}

bool media_output_can_display_videos(void) {
    if (!io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    return notcurses_canopen_videos(io_handler_ref->nc);
}