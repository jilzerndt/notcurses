#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <notcurses/notcurses.h>
#include "../io_handler.h"

/**
 * Key types for game input
 */
typedef enum key_type {
    KEY_TYPE_UNKNOWN = 0,
    KEY_TYPE_ARROW,        // Arrow keys (up, down, left, right)
    KEY_TYPE_ACTION,       // Action keys (enter, space, etc.)
    KEY_TYPE_CHAR,         // Character keys (a-z, 0-9, etc.)
    KEY_TYPE_FUNCTION,     // Function keys (F1-F12)
    KEY_TYPE_MODIFIER,     // Modifier keys (ctrl, alt, shift)
    KEY_TYPE_ESCAPE,       // Escape key
    KEY_TYPE_QUIT          // Quit keys (q, Ctrl+C, etc.)
} key_type_t;

/**
 * Direction values for arrow keys
 */
typedef enum key_direction {
    DIRECTION_NONE = 0,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} key_direction_t;

/**
 * Action key types
 */
typedef enum key_action {
    ACTION_NONE = 0,
    ACTION_CONFIRM,        // Enter, Space
    ACTION_CANCEL,         // Backspace, Delete
    ACTION_MENU,           // Tab, Escape when used for menu
    ACTION_INTERACT,       // 'e', 'f', etc. based on configuration
    ACTION_ATTACK,         // 'a', etc.
    ACTION_USE_ITEM,       // 'i', etc.
    ACTION_HELP            // 'h', '?', etc.
} key_action_t;

/**
 * Input event structure
 */
typedef struct input_event {
    key_type_t type;           // Type of key pressed
    union {
        key_direction_t direction; // Direction if arrow key
        key_action_t action;       // Action if action key
        char character;           // Character if character key
        int function_key;         // Function key number (1-12)
    } value;
    bool alt_pressed;           // Alt modifier
    bool ctrl_pressed;          // Ctrl modifier
    bool shift_pressed;         // Shift modifier
} input_event_t;

/**
 * Input handler configuration structure
 */
typedef struct input_config {
    bool block_input;            // Should input block (wait for key)
    int timeout_ms;              // Timeout in milliseconds (for non-blocking)
    bool mouse_enabled;          // Mouse input enabled
    bool quit_on_escape;         // Quit on escape key
    bool use_wasd;               // Use WASD for movement (true) or arrows (false)
    char key_interact;           // Key for interaction
    char key_attack;             // Key for attack
    char key_use_item;           // Key for using items
    char key_help;               // Key for help
} input_config_t;

/**
 * Input handler structure
 */
typedef struct input_handler {
    input_config_t config;       // Input configuration
    uint32_t last_key;           // Last key pressed
    input_event_t last_event;    // Last input event
    bool has_pending_input;      // Flag for pending input
} input_handler_t;

/**
 * Initialize the input handler
 * @param io_handler Pointer to the IO handler
 * @return Pointer to the input handler or NULL on failure
 */
input_handler_t* input_handler_init(io_handler_t* io_handler);

/**
 * Initialize the input handler with custom configuration
 * @param io_handler Pointer to the IO handler
 * @param config Custom input configuration
 * @return Pointer to the input handler or NULL on failure
 */
input_handler_t* input_handler_init_with_config(io_handler_t* io_handler, 
                                              input_config_t config);

/**
 * Get default input configuration
 * @return Default input configuration
 */
input_config_t input_handler_default_config(void);

/**
 * Clean up the input handler resources
 * @param handler Pointer to the input handler
 */
void input_handler_cleanup(input_handler_t* handler);

/**
 * Poll for input (non-blocking)
 * @param handler Pointer to the input handler
 * @param io_handler Pointer to the IO handler
 * @param event Pointer to store the input event
 * @return true if input was received, false otherwise
 */
bool input_handler_poll(input_handler_t* handler, io_handler_t* io_handler, 
                       input_event_t* event);

/**
 * Wait for input (blocking)
 * @param handler Pointer to the input handler
 * @param io_handler Pointer to the IO handler
 * @param event Pointer to store the input event
 * @return true on success, false on error
 */
bool input_handler_wait(input_handler_t* handler, io_handler_t* io_handler, 
                       input_event_t* event);

/**
 * Wait for input with timeout
 * @param handler Pointer to the input handler
 * @param io_handler Pointer to the IO handler
 * @param event Pointer to store the input event
 * @param timeout_ms Timeout in milliseconds
 * @return true if input was received, false on timeout or error
 */
bool input_handler_wait_timeout(input_handler_t* handler, io_handler_t* io_handler, 
                              input_event_t* event, int timeout_ms);

/**
 * Check if a quit key was pressed
 * @param event Input event to check
 * @return true if it's a quit key, false otherwise
 */
bool input_handler_is_quit(const input_event_t* event);

/**
 * Enable or disable mouse input
 * @param handler Pointer to the input handler
 * @param io_handler Pointer to the IO handler
 * @param enabled true to enable, false to disable
 * @return true on success, false on error
 */
bool input_handler_set_mouse(input_handler_t* handler, io_handler_t* io_handler, 
                            bool enabled);

/**
 * Convert a raw key to an input event
 * @param handler Pointer to the input handler
 * @param key Raw key code
 * @param raw_event Raw input event from notcurses
 * @param event Pointer to store the input event
 */
void input_handler_convert_key(input_handler_t* handler, uint32_t key, 
                              const ncinput* raw_event, input_event_t* event);

/**
 * Check if the terminal supports mouse input
 * @param io_handler Pointer to the IO handler
 * @return true if supported, false otherwise
 */
bool input_handler_mouse_supported(io_handler_t* io_handler);

#endif /* INPUT_HANDLER_H */