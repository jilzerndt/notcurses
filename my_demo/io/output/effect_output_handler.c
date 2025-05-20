#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "effect_output_handler.h"
#include "../io_handler.h"

// Reference to the IO handler
static io_handler_t* io_handler_ref = NULL;

// Linked list of active effects
static effect_state_t* active_effects = NULL;

// Linear interpolation helper
static uint8_t lerp_value(uint8_t start, uint8_t end, float progress) {
    return (uint8_t)(start + (end - start) * progress);
}

// Color interpolation for fade effects
static uint64_t interpolate_channels(uint64_t start, uint64_t end, float progress) {
    uint64_t result = 0;
    
    // Extract RGB components from start channels
    uint8_t sr, sg, sb, er, eg, eb;
    ncchannels_fg_rgb8(start, &sr, &sg, &sb);
    ncchannels_bg_rgb8(start, &sr, &sg, &sb);
    
    // Extract RGB components from end channels
    ncchannels_fg_rgb8(end, &er, &eg, &eb);
    ncchannels_bg_rgb8(end, &er, &eg, &eb);
    
    // Interpolate between the colors
    uint8_t r = lerp_value(sr, er, progress);
    uint8_t g = lerp_value(sg, eg, progress);
    uint8_t b = lerp_value(sb, eb, progress);
    
    uint8_t br = lerp_value(sr, er, progress);
    uint8_t bg = lerp_value(sg, eg, progress);
    uint8_t bb = lerp_value(sb, eb, progress);
    
    // Set the new channel values
    NCCHANNELS_SET_FG_RGB8(&result, r, g, b);
    NCCHANNELS_SET_BG_RGB8(&result, br, bg, bb);
    
    return result;
}

// Add an effect to the active effects list
static bool add_effect(effect_state_t* effect) {
    if (!effect) {
        return false;
    }
    
    // Add to the front of the list
    effect->next = active_effects;
    active_effects = effect;
    return true;
}

// Remove an effect from the active effects list
static void remove_effect(effect_state_t* effect) {
    if (!effect || !active_effects) {
        return;
    }
    
    // Check if it's the first element
    if (active_effects == effect) {
        active_effects = effect->next;
        free(effect);
        return;
    }
    
    // Find the effect in the list
    effect_state_t* current = active_effects;
    while (current->next) {
        if (current->next == effect) {
            current->next = effect->next;
            free(effect);
            return;
        }
        current = current->next;
    }
}

bool effect_output_init(void) {
    // This will be initialized when the IO handler is created
    active_effects = NULL;
    return true;
}

void effect_output_cleanup(void) {
    // Free all active effects
    effect_state_t* current = active_effects;
    while (current) {
        effect_state_t* next = current->next;
        free(current);
        current = next;
    }
    active_effects = NULL;
}

bool effect_output_supported(void) {
    if (!io_handler_ref || !io_handler_ref->nc) {
        return false;
    }
    // Check if fade effects are supported
    return notcurses_canfade(io_handler_ref->nc);
}

bool effect_output_fade(struct ncplane* plane, int duration_ms,
                      output_color_t from_fg, output_color_t from_bg,
                      output_color_t to_fg, output_color_t to_bg) {
    if (!plane || duration_ms <= 0 || !effect_output_supported()) {
        return false;
    }
    
    // Create a new effect state
    effect_state_t* effect = malloc(sizeof(effect_state_t));
    if (!effect) {
        return false;
    }
    
    // Initialize the effect
    effect->type = EFFECT_FADE;
    effect->plane = plane;
    effect->duration_ms = duration_ms;
    effect->elapsed_ms = 0;
    effect->active = true;
    effect->next = NULL;
    
    // Set start channels
    output_handler_set_fg_color(from_fg, &effect->start_channels);
    output_handler_set_bg_color(from_bg, &effect->start_channels);
    
    // Set end channels
    output_handler_set_fg_color(to_fg, &effect->end_channels);
    output_handler_set_bg_color(to_bg, &effect->end_channels);
    
    // Apply the initial state
    ncplane_set_channels(plane, effect->start_channels);
    
    // Add to active effects
    return add_effect(effect);
}

bool effect_output_pulse(struct ncplane* plane, int duration_ms,
                       output_color_t fg1, output_color_t bg1,
                       output_color_t fg2, output_color_t bg2,
                       int cycles) {
    if (!plane || duration_ms <= 0 || !effect_output_supported()) {
        return false;
    }
    
    // Create a new effect state
    effect_state_t* effect = malloc(sizeof(effect_state_t));
    if (!effect) {
        return false;
    }
    
    // Initialize the effect
    effect->type = EFFECT_PULSE;
    effect->plane = plane;
    effect->duration_ms = duration_ms;
    effect->elapsed_ms = 0;
    effect->active = true;
    effect->next = NULL;
    
    // Set start channels (for first color)
    output_handler_set_fg_color(fg1, &effect->start_channels);
    output_handler_set_bg_color(bg1, &effect->start_channels);
    
    // Set end channels (for second color)
    output_handler_set_fg_color(fg2, &effect->end_channels);
    output_handler_set_bg_color(bg2, &effect->end_channels);
    
    // Apply the initial state
    ncplane_set_channels(plane, effect->start_channels);
    
    // Add to active effects
    return add_effect(effect);
}

bool effect_output_flash(struct ncplane* plane, int duration_ms,
                       output_color_t color) {
    if (!plane || duration_ms <= 0 || !effect_output_supported()) {
        return false;
    }
    
    // Create a new effect state
    effect_state_t* effect = malloc(sizeof(effect_state_t));
    if (!effect) {
        return false;
    }
    
    // Initialize the effect
    effect->type = EFFECT_FLASH;
    effect->plane = plane;
    effect->duration_ms = duration_ms;
    effect->elapsed_ms = 0;
    effect->active = true;
    effect->next = NULL;
    
    // Save current channels as the start channels
    effect->start_channels = ncplane_channels(plane);
    
    // Set end channels (full flash color)
    output_handler_set_fg_color(color, &effect->end_channels);
    output_handler_set_bg_color(color, &effect->end_channels);
    
    // Add to active effects
    return add_effect(effect);
}

bool effect_output_blink(struct ncplane* plane, int interval_ms, int count) {
    if (!plane || interval_ms <= 0 || !effect_output_supported()) {
        return false;
    }
    
    // Create a new effect state
    effect_state_t* effect = malloc(sizeof(effect_state_t));
    if (!effect) {
        return false;
    }
    
    // Initialize the effect
    effect->type = EFFECT_BLINK;
    effect->plane = plane;
    effect->duration_ms = interval_ms * (count > 0 ? count * 2 : 1000); // Long duration for infinite
    effect->elapsed_ms = 0;
    effect->active = true;
    effect->next = NULL;
    
    // Save current visibility state
    effect->start_channels = ncplane_channels(plane);
    
    // Add to active effects
    return add_effect(effect);
}

bool effect_output_update(int elapsed_ms) {
    if (!active_effects || elapsed_ms <= 0) {
        return false;
    }
    
    bool updated = false;
    effect_state_t* current = active_effects;
    effect_state_t* next = NULL;
    
    while (current) {
        // Save next pointer as we might free current
        next = current->next;
        
        if (current->active) {
            current->elapsed_ms += elapsed_ms;
            float progress = (float)current->elapsed_ms / current->duration_ms;
            
            // Handle based on effect type
            switch (current->type) {
                case EFFECT_FADE: {
                    if (progress >= 1.0f) {
                        // Apply final state and mark for removal
                        ncplane_set_channels(current->plane, current->end_channels);
                        current->active = false;
                    } else {
                        // Interpolate between start and end channels
                        uint64_t channels = interpolate_channels(
                            current->start_channels, current->end_channels, progress);
                        ncplane_set_channels(current->plane, channels);
                    }
                    updated = true;
                    break;
                }
                
                case EFFECT_PULSE: {
                    // Use a sine wave to create smooth pulsing
                    float pulse = (sinf(progress * M_PI * 2) + 1) / 2.0f;
                    uint64_t channels = interpolate_channels(
                        current->start_channels, current->end_channels, pulse);
                    ncplane_set_channels(current->plane, channels);
                    
                    // For pulse, we keep going until manually stopped
                    if (progress >= 1.0f) {
                        current->elapsed_ms %= current->duration_ms;
                    }
                    updated = true;
                    break;
                }
                
                case EFFECT_FLASH: {
                    if (progress >= 1.0f) {
                        // Return to original color and mark for removal
                        ncplane_set_channels(current->plane, current->start_channels);
                        current->active = false;
                    } else {
                        // Use triangle wave for flash (up then down)
                        float flash_progress = progress < 0.5f 
                            ? progress * 2   // 0 -> 1 (first half)
                            : (1.0f - progress) * 2;  // 1 -> 0 (second half)
                        
                        uint64_t channels = interpolate_channels(
                            current->start_channels, current->end_channels, flash_progress);
                        ncplane_set_channels(current->plane, channels);
                    }
                    updated = true;
                    break;
                }
                
                case EFFECT_BLINK: {
                    // Toggle visibility every half cycle
                    bool visible = (current->elapsed_ms / (current->duration_ms / 20)) % 2 == 0;
                    if (visible) {
                        ncplane_set_channels(current->plane, current->start_channels);
                    } else {
                        // Make invisible by setting alpha
                        uint64_t invisible = current->start_channels;
                        NCCHANNELS_SET_FG_ALPHA(&invisible, NCALPHA_TRANSPARENT);
                        NCCHANNELS_SET_BG_ALPHA(&invisible, NCALPHA_TRANSPARENT);
                        ncplane_set_channels(current->plane, invisible);
                    }
                    
                    // For blink, we keep going until manually stopped or count is reached
                    if (progress >= 1.0f) {
                        // Make sure it's visible at the end
                        ncplane_set_channels(current->plane, current->start_channels);
                        current->active = false;
                    }
                    updated = true;
                    break;
                }
            }
            
            // Remove completed effects
            if (!current->active) {
                remove_effect(current);
            }
        }
        
        current = next;
    }
    
    return updated;
}

void effect_output_stop_all(struct ncplane* plane) {
    if (!plane || !active_effects) {
        return;
    }
    
    effect_state_t* current = active_effects;
    effect_state_t* prev = NULL;
    
    while (current) {
        if (current->plane == plane) {
            effect_state_t* to_remove = current;
            
            // Handle list linkage
            if (prev) {
                prev->next = current->next;
                current = current->next;
            } else {
                active_effects = current->next;
                current = active_effects;
            }
            
            // Free the removed effect
            free(to_remove);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void effect_output_stop(struct ncplane* plane, effect_type_t type) {
    if (!plane || !active_effects) {
        return;
    }
    
    effect_state_t* current = active_effects;
    effect_state_t* prev = NULL;
    
    while (current) {
        if (current->plane == plane && current->type == type) {
            effect_state_t* to_remove = current;
            
            // Handle list linkage
            if (prev) {
                prev->next = current->next;
                current = current->next;
            } else {
                active_effects = current->next;
                current = active_effects;
            }
            
            // Free the removed effect
            free(to_remove);
        } else {
            prev = current;
            current = current->next;
        }
    }
}