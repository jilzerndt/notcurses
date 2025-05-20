#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "input_handler.h"

// Global reference to the input handler (singleton)
static input_handler_t* input_handler_instance = NULL;

input_config_t input_handler_default_config(void) {
    input_config_t config = {
        .block_input = true,         // Block for input by default
        .timeout_ms = 100,           // 100ms timeout for non-blocking input
        .mouse_enabled = false,      // Mouse disabled by default
        .quit_on_escape = true,      // Quit on escape key
        .use_wasd = true,            // Use WASD for movement
        .key_interact = 'e',         // 'e' for interact
        .key_attack = 'a',           // 'a' for attack
        .key_use_item = 'i',         // 'i' for items
        .key_help = 'h'              // 'h' for help
    };
    
    return config;
}

input_handler_t* input_handler_init(io_handler_t* io_handler) {
    // Use default configuration
    return input_handler_init_with_config(io_handler, input_handler_default_config());
}

input_handler_t* input_handler_init_with_config(io_handler_t* io_handler, 
                                              input_config_t config) {
    if (!io_handler || !io_handler->nc) {
        return NULL;
    }

    // Create singleton instance if not already created
    if (!input_handler_instance) {
        input_handler_instance = malloc(sizeof(input_handler_t));
        if (!input_handler_instance) {
            fprintf(stderr, "Failed to allocate memory for input handler\n");
            return NULL;
        }
    }

    // Initialize with provided configuration
    input_handler_instance->config = config;
    input_handler_instance->last_key = 0;
    input_handler_instance->has_pending_input = false;
    memset(&input_handler_instance->last_event, 0, sizeof(input_event_t));

    // Enable mouse if configured and supported
    if (config.mouse_enabled && input_handler_mouse_supported(io_handler)) {
        if (notcurses_mice_enable(io_handler->nc, NCMICE_ALL_EVENTS) == -1) {
            fprintf(stderr, "Warning: Failed to enable mouse support\n");
            input_handler_instance->config.mouse_enabled = false;
        }
    }

    return input_handler_instance;
}

void input_handler_cleanup(input_handler_t* handler) {
    if (handler == input_handler_instance) {
        input_handler_instance = NULL;
    }
    
    free(handler);
}

void input_handler_convert_key(input_handler_t* handler, uint32_t key, 
                              const ncinput* raw_event, input_event_t* event) {
    if (!handler || !event) {
        return;
    }

    // Initialize event
    memset(event, 0, sizeof(input_event_t));
    
    // Set modifier keys if available
    if (raw_event) {
        event->alt_pressed = raw_event->alt;
        event->ctrl_pressed = raw_event->ctrl;
        event->shift_pressed = raw_event->shift;
    }

    // Check for special keys
    switch (key) {
        // Arrow keys
        case NCKEY_UP:
            event->type = KEY_TYPE_ARROW;
            event->value.direction = DIRECTION_UP;
            break;
        case NCKEY_DOWN:
            event->type = KEY_TYPE_ARROW;
            event->value.direction = DIRECTION_DOWN;
            break;
        case NCKEY_LEFT:
            event->type = KEY_TYPE_ARROW;
            event->value.direction = DIRECTION_LEFT;
            break;
        case NCKEY_RIGHT:
            event->type = KEY_TYPE_ARROW;
            event->value.direction = DIRECTION_RIGHT;
            break;
            
        // Action keys
        case NCKEY_ENTER:
        case ' ':
            event->type = KEY_TYPE_ACTION;
            event->value.action = ACTION_CONFIRM;
            break;
        case NCKEY_BACKSPACE:
        case NCKEY_DEL:
            event->type = KEY_TYPE_ACTION;
            event->value.action = ACTION_CANCEL;
            break;
        case NCKEY_TAB:
            event->type = KEY_TYPE_ACTION;
            event->value.action = ACTION_MENU;
            break;
            
        // Function keys
        case NCKEY_F01:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 1;
            break;
        case NCKEY_F02:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 2;
            break;
        case NCKEY_F03:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 3;
            break;
        case NCKEY_F04:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 4;
            break;
        case NCKEY_F05:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 5;
            break;
        case NCKEY_F06:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 6;
            break;
        case NCKEY_F07:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 7;
            break;
        case NCKEY_F08:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 8;
            break;
        case NCKEY_F09:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 9;
            break;
        case NCKEY_F10:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 10;
            break;
        case NCKEY_F11:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 11;
            break;
        case NCKEY_F12:
            event->type = KEY_TYPE_FUNCTION;
            event->value.function_key = 12;
            break;
            
        // Escape key
        case NCKEY_ESC:
            event->type = KEY_TYPE_ESCAPE;
            break;
            
        // Check for regular characters
        default:
            // Check if it's a printable ASCII character
            if (key >= 32 && key <= 126) {
                event->type = KEY_TYPE_CHAR;
                event->value.character = (char)key;
                
                // Check for WASD movement if enabled
                if (handler->config.use_wasd) {
                    char c = (char)key;
                    if (c == 'w' || c == 'W') {
                        event->type = KEY_TYPE_ARROW;
                        event->value.direction = DIRECTION_UP;
                    } else if (c == 'a' || c == 'A') {
                        event->type = KEY_TYPE_ARROW;
                        event->value.direction = DIRECTION_LEFT;
                    } else if (c == 's' || c == 'S') {
                        event->type = KEY_TYPE_ARROW;
                        event->value.direction = DIRECTION_DOWN;
                    } else if (c == 'd' || c == 'D') {
                        event->type = KEY_TYPE_ARROW;
                        event->value.direction = DIRECTION_RIGHT;
                    }
                }
                
                // Check for action keys
                char c = (char)key;
                if (c == handler->config.key_interact || 
                    toupper(c) == toupper(handler->config.key_interact)) {
                    event->type = KEY_TYPE_ACTION;
                    event->value.action = ACTION_INTERACT;
                } else if (c == handler->config.key_attack || 
                          toupper(c) == toupper(handler->config.key_attack)) {
                    event->type = KEY_TYPE_ACTION;
                    event->value.action = ACTION_ATTACK;
                } else if (c == handler->config.key_use_item || 
                          toupper(c) == toupper(handler->config.key_use_item)) {
                    event->type = KEY_TYPE_ACTION;
                    event->value.action = ACTION_USE_ITEM;
                } else if (c == handler->config.key_help || 
                          toupper(c) == toupper(handler->config.key_help) || 
                          c == '?') {
                    event->type = KEY_TYPE_ACTION;
                    event->value.action = ACTION_HELP;
                }
                
                // Check for quit keys
                if ((c == 'q' || c == 'Q') && event->ctrl_pressed) {
                    event->type = KEY_TYPE_QUIT;
                }
            } else {
                event->type = KEY_TYPE_UNKNOWN;
            }
            break;
    }
    
    // Check for escape as quit
    if (event->type == KEY_TYPE_ESCAPE && handler->config.quit_on_escape) {
        event->type = KEY_TYPE_QUIT;
    }
}

bool input_handler_poll(input_handler_t* handler, io_handler_t* io_handler, 
                       input_event_t* event) {
    if (!handler || !io_handler || !io_handler->nc || !event) {
        return false;
    }
    
    // Get input with zero timeout (non-blocking)
    struct timespec ts = {0, 0};
    ncinput raw_event;
    
    uint32_t key = notcurses_get(io_handler->nc, &ts, &raw_event);
    if (key == 0) {
        // No input available
        return false;
    }
    
    // Convert raw key to input event
    input_handler_convert_key(handler, key, &raw_event, event);
    
    // Store last key and event
    handler->last_key = key;
    handler->last_event = *event;
    handler->has_pending_input = true;
    
    return true;
}

bool input_handler_wait(input_handler_t* handler, io_handler_t* io_handler, 
                       input_event_t* event) {
    if (!handler || !io_handler || !io_handler->nc || !event) {
        return false;
    }
    
    // Get input with no timeout (blocking)
    ncinput raw_event;
    
    uint32_t key = notcurses_get_blocking(io_handler->nc, &raw_event);
    if (key == (uint32_t)-1) {
        // Error occurred
        return false;
    }
    
    // Convert raw key to input event
    input_handler_convert_key(handler, key, &raw_event, event);
    
    // Store last key and event
    handler->last_key = key;
    handler->last_event = *event;
    handler->has_pending_input = true;
    
    return true;
}

bool input_handler_wait_timeout(input_handler_t* handler, io_handler_t* io_handler, 
                              input_event_t* event, int timeout_ms) {
    if (!handler || !io_handler || !io_handler->nc || !event || timeout_ms < 0) {
        return false;
    }
    
    // Get input with specified timeout
    struct timespec ts;
    ts.tv_sec = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000000;
    
    ncinput raw_event;
    
    uint32_t key = notcurses_get(io_handler->nc, &ts, &raw_event);
    if (key == 0) {
        // Timeout occurred
        return false;
    }
    
    if (key == (uint32_t)-1) {
        // Error occurred
        return false;
    }
    
    // Convert raw key to input event
    input_handler_convert_key(handler, key, &raw_event, event);
    
    // Store last key and event
    handler->last_key = key;
    handler->last_event = *event;
    handler->has_pending_input = true;
    
    return true;
}

bool input_handler_is_quit(const input_event_t* event) {
    if (!event) {
        return false;
    }
    
    return event->type == KEY_TYPE_QUIT;
}

bool input_handler_set_mouse(input_handler_t* handler, io_handler_t* io_handler, 
                            bool enabled) {
    if (!handler || !io_handler || !io_handler->nc) {
        return false;
    }
    
    if (enabled) {
        // Try to enable mouse input
        if (notcurses_mice_enable(io_handler->nc, NCMICE_ALL_EVENTS) == -1) {
            return false;
        }
    } else {
        // Disable mouse input
        if (notcurses_mice_disable(io_handler->nc) == -1) {
            return false;
        }
    }
    
    handler->config.mouse_enabled = enabled;
    return true;
}

bool input_handler_mouse_supported(io_handler_t* io_handler) {
    if (!io_handler || !io_handler->nc) {
        return false;
    }
    
    // Try to enable mouse input then disable it to test support
    int result = notcurses_mice_enable(io_handler->nc, NCMICE_ALL_EVENTS);
    if (result == -1) {
        return false;
    }
    
    notcurses_mice_disable(io_handler->nc);
    return true;
}