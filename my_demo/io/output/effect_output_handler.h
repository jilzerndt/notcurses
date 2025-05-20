#ifndef EFFECT_OUTPUT_HANDLER_H
#define EFFECT_OUTPUT_HANDLER_H

#include <stdbool.h>
#include <notcurses/notcurses.h>
#include "output_handler.h"

/**
 * Effect types for visual enhancements
 */
typedef enum effect_type {
    EFFECT_FADE,     // Fade in/out
    EFFECT_PULSE,    // Pulse colors
    EFFECT_FLASH,    // Flash briefly
    EFFECT_BLINK     // Toggle visibility
} effect_type_t;

/**
 * Effect state for tracking effect progress
 */
typedef struct effect_state {
    effect_type_t type;        // Type of effect
    struct ncplane* plane;     // Target plane
    int duration_ms;           // Total duration in milliseconds
    int elapsed_ms;            // Elapsed time in milliseconds
    uint64_t start_channels;   // Starting channels
    uint64_t end_channels;     // Ending channels (for fades)
    bool active;               // Is the effect currently active
    struct effect_state* next; // For linked list of effects
} effect_state_t;

/**
 * Initialize the effect system
 * @return true on success, false on failure
 */
bool effect_output_init(void);

/**
 * Clean up the effect system resources
 */
void effect_output_cleanup(void);

/**
 * Check if effects are supported on this terminal
 * @return true if effects are supported, false otherwise
 */
bool effect_output_supported(void);

/**
 * Create a fade effect from one color to another
 * @param plane Target plane to apply effect
 * @param duration_ms Duration of the effect in milliseconds
 * @param from_fg Starting foreground color
 * @param from_bg Starting background color
 * @param to_fg Ending foreground color
 * @param to_bg Ending background color
 * @return true on success, false on failure
 */
bool effect_output_fade(struct ncplane* plane, int duration_ms,
                        output_color_t from_fg, output_color_t from_bg,
                        output_color_t to_fg, output_color_t to_bg);

/**
 * Create a pulse effect that transitions between two colors
 * @param plane Target plane to apply effect
 * @param duration_ms Duration of one pulse cycle in milliseconds
 * @param fg1 First foreground color
 * @param bg1 First background color
 * @param fg2 Second foreground color
 * @param bg2 Second background color
 * @param cycles Number of pulse cycles (0 for infinite)
 * @return true on success, false on failure
 */
bool effect_output_pulse(struct ncplane* plane, int duration_ms,
                         output_color_t fg1, output_color_t bg1,
                         output_color_t fg2, output_color_t bg2,
                         int cycles);

/**
 * Create a flash effect (bright flash then return to normal)
 * @param plane Target plane to apply effect
 * @param duration_ms Duration of the effect in milliseconds
 * @param color Flash color (usually white or yellow)
 * @return true on success, false on failure
 */
bool effect_output_flash(struct ncplane* plane, int duration_ms,
                         output_color_t color);

/**
 * Create a blink effect (toggle visibility)
 * @param plane Target plane to apply effect
 * @param interval_ms Time between blinks in milliseconds
 * @param count Number of blinks (0 for infinite)
 * @return true on success, false on failure
 */
bool effect_output_blink(struct ncplane* plane, int interval_ms, int count);

/**
 * Update all active effects
 * @param elapsed_ms Milliseconds since last update
 * @return true if any effects were updated, false otherwise
 */
bool effect_output_update(int elapsed_ms);

/**
 * Stop all active effects on a plane
 * @param plane Target plane
 */
void effect_output_stop_all(struct ncplane* plane);

/**
 * Stop a specific effect type on a plane
 * @param plane Target plane
 * @param type Effect type to stop
 */
void effect_output_stop(struct ncplane* plane, effect_type_t type);

#endif /* EFFECT_OUTPUT_HANDLER_H */