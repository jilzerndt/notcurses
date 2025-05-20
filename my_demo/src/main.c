#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../io/io_handler.h"
#include "../io/output/output_handler.h"
#include "../io/output/text_output_handler.h"
#include "../io/output/media_output_handler.h"
#include "../io/output/effect_output_handler.h"
#include "../io/output/fancy_output_handler.h"
#include "../io/input/input_handler.h"

// Cross-platform sleep function
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

// Example menu callback function
void menu_callback(int id, void* user_data) {
    // In a real game, this would handle menu selection
    (void)user_data; // Avoid unused parameter warning
    
    // We don't printf in a real terminal app as it will mess with display
    // Instead, the callback would modify game state in a real implementation
}

// Determine if we should use Unicode characters based on platform and terminal support
bool should_use_unicode(io_handler_t* io) {
    if (!io) {
        return false;
    }
    
    // Check if Unicode is supported by the terminal
    if (io_handler_supports_unicode(io)) {
        return true;
    }
    
    // On Windows, it depends on the terminal but modern Windows Terminal supports it
    if (io->platform == PLATFORM_TYPE_WINDOWS) {
        char* term_program = getenv("WT_SESSION"); // Windows Terminal
        if (term_program) {
            return true;
        }
    }
    
    return false;
}

int main(void) {
    // Initialize the IO handler
    io_handler_t* io = io_handler_init();
    if (!io) {
        fprintf(stderr, "Failed to initialize IO handler\n");
        return 1;
    }

    // Get output handler instance
    output_handler_t* output = output_handler_get();
    if (!output) {
        fprintf(stderr, "Failed to get output handler\n");
        io_handler_cleanup(io);
        return 1;
    }
    
    // Initialize input handler
    input_handler_t* input = input_handler_init(io);
    if (!input) {
        fprintf(stderr, "Failed to initialize input handler\n");
        io_handler_cleanup(io);
        return 1;
    }

    // Display platform information
    const char* platform_name = "Unknown";
    switch (io->platform) {
        case PLATFORM_TYPE_WINDOWS: platform_name = "Windows"; break;
        case PLATFORM_TYPE_MACOS: platform_name = "macOS"; break;
        case PLATFORM_TYPE_LINUX: platform_name = "Linux"; break;
        default: platform_name = "Unknown"; break;
    }

    // Draw welcome message with platform info
    text_output_print_message(COLOR_GREEN, COLOR_BLACK, TEXT_STYLE_BOLD, 
                             "Welcome to Dungeon Crawl (%s)! Press any key to continue...",
                             platform_name);
    io_handler_render(io);
    
    // Wait for keypress using our input handler
    input_event_t event;
    input_handler_wait(input, io, &event);
    
    // Clear the message
    text_output_clear_message(COLOR_WHITE, COLOR_BLACK, TEXT_STYLE_NORMAL, 
                             "Initializing game...");
    io_handler_render(io);
    
    // Check if we should use Unicode characters
    bool use_unicode = should_use_unicode(io);
    box_style_t box_style = use_unicode ? BOX_STYLE_DOUBLE : BOX_STYLE_ASCII;
    
    // Draw a game title with fancy box
    fancy_output_draw_titled_box(output->game_plane, 2, 10, 5, 40, box_style,
                                COLOR_CYAN, COLOR_BLACK, "DUNGEON CRAWL");
    
    // Display some text in the game area
    text_output_print_game(4, 15, COLOR_WHITE, COLOR_BLACK, TEXT_STYLE_BOLD,
                          "An 80s-style terminal adventure!");
    
    // Show platform-specific instructions
    const char* movement_keys = input->config.use_wasd ? "WASD" : "Arrow Keys";
    text_output_print_game(6, 15, COLOR_WHITE, COLOR_BLACK, TEXT_STYLE_NORMAL,
                          "Platform: %s - Use %s to move", platform_name, movement_keys);
    
    // Draw UI elements in the UI plane
    fancy_output_draw_box(output->ui_plane, 0, 0, 3, 30, box_style,
                         COLOR_BLUE, COLOR_BLACK);
    text_output_print(output->ui_plane, 1, 2, COLOR_YELLOW, COLOR_BLACK, TEXT_STYLE_BOLD,
                     "Health: ");
    
    // Draw a health bar
    fancy_output_draw_progress_bar(output->ui_plane, 1, 10, 15, 75,
                                  COLOR_RED, COLOR_GREEN);
    
    // Create a custom plane for a menu
    struct ncplane* menu_plane = output_handler_create_plane(10, 15, 10, 30, 1);
    
    // Create some menu items
    menu_item_t items[] = {
        {"Start New Game", 1, true},
        {"Load Game", 2, true},
        {"Options", 3, true},
        {"Quit", 4, true}
    };
    
    // Create a menu
    menu_t* menu = fancy_output_create_menu(10, 15, items, 4, "Main Menu",
                                         COLOR_WHITE, COLOR_BLUE, menu_callback, NULL);
    
    // Render everything
    io_handler_render(io);
    
    // Main loop demonstration (would be part of the game loop)
    int frames = 0;
    const int max_frames = 30; // Run for a short time
    bool running = true;
    
    while (running && frames < max_frames) {
        // Process input (non-blocking)
        if (input_handler_poll(input, io, &event)) {
            // Handle input
            if (input_handler_is_quit(&event)) {
                running = false;
                break;
            }
            
            // Process menu input if it's an arrow, action, or character key
            if (event.type == KEY_TYPE_ARROW || 
                event.type == KEY_TYPE_ACTION || 
                event.type == KEY_TYPE_CHAR) {
                fancy_output_menu_input(menu, input->last_key);
            }
        }
        
        // Update effects (if any)
        effect_output_update(100); // 100ms per frame
        
        // Message with frame counter
        text_output_clear_message(COLOR_WHITE, COLOR_BLACK, TEXT_STYLE_NORMAL,
                                "Demo running... Frame %d/%d (Press Q to quit)", 
                                frames + 1, max_frames);
        
        // Apply a pulsing effect to the title box
        if (frames == 5) {
            effect_output_pulse(menu_plane, 1000, COLOR_WHITE, COLOR_BLUE,
                              COLOR_YELLOW, COLOR_BLUE, 10);
        }
        
        // Render the frame
        io_handler_render(io);
        frames++;
        
        // Cross-platform sleep
        sleep_ms(100);
    }
    
    // Clean up and exit
    if (menu) {
        fancy_output_destroy_menu(menu);
    }
    
    // Display exit message
    text_output_clear_message(COLOR_GREEN, COLOR_BLACK, TEXT_STYLE_BOLD,
                            "Demo completed. Press any key to exit...");
    io_handler_render(io);
    
    // Wait for key press using input handler
    input_handler_wait(input, io, &event);
    
    // Clean up resources
    input_handler_cleanup(input);
    io_handler_cleanup(io);
    return 0;
}