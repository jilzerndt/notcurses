#ifndef AUDIO_OUTPUT_HANDLER_H
#define AUDIO_OUTPUT_HANDLER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Audio format types supported
 */
typedef enum audio_format {
    AUDIO_FORMAT_UNKNOWN = 0,
    AUDIO_FORMAT_WAV,    // WAV format
    AUDIO_FORMAT_MP3,    // MP3 format
    AUDIO_FORMAT_OGG     // OGG Vorbis format
} audio_format_t;

/**
 * Audio instance structure
 */
typedef struct audio_instance {
    void* handle;            // Backend-specific handle
    char* filename;          // Path to audio file
    bool is_playing;         // Is audio currently playing
    bool is_looping;         // Should audio loop when finished
    float volume;            // Volume level (0.0 - 1.0)
    int channel;             // Audio channel (for mixing)
    audio_format_t format;   // Audio format
} audio_instance_t;

/**
 * Initialize the audio system
 * @return true on success, false on failure
 */
bool audio_output_init(void);

/**
 * Clean up the audio system
 */
void audio_output_cleanup(void);

/**
 * Check if audio is supported
 * @return true if audio is supported, false otherwise
 */
bool audio_output_supported(void);

/**
 * Load an audio file and prepare it for playback
 * @param filename Path to the audio file
 * @param channel Audio channel to use (0-31, or -1 for auto)
 * @return Pointer to audio instance or NULL on failure
 */
audio_instance_t* audio_output_load(const char* filename, int channel);

/**
 * Play an audio file
 * @param filename Path to the audio file
 * @param volume Volume level (0.0 - 1.0)
 * @param loop Whether to loop the audio (true) or play once (false)
 * @return Pointer to audio instance or NULL on failure
 */
audio_instance_t* audio_output_play(const char* filename, float volume, bool loop);

/**
 * Play a sound effect (non-looping audio)
 * @param filename Path to the audio file
 * @param volume Volume level (0.0 - 1.0)
 * @return true on success, false on failure
 */
bool audio_output_play_effect(const char* filename, float volume);

/**
 * Start or resume playback of an already loaded audio instance
 * @param audio Pointer to the audio instance
 * @return true on success, false on failure
 */
bool audio_output_start(audio_instance_t* audio);

/**
 * Pause playback of an audio instance
 * @param audio Pointer to the audio instance
 * @return true on success, false on failure
 */
bool audio_output_pause(audio_instance_t* audio);

/**
 * Stop playback of an audio instance and reset position to beginning
 * @param audio Pointer to the audio instance
 * @return true on success, false on failure
 */
bool audio_output_stop(audio_instance_t* audio);

/**
 * Set the volume of an audio instance
 * @param audio Pointer to the audio instance
 * @param volume Volume level (0.0 - 1.0)
 * @return true on success, false on failure
 */
bool audio_output_set_volume(audio_instance_t* audio, float volume);

/**
 * Set whether an audio instance should loop
 * @param audio Pointer to the audio instance
 * @param loop Whether to loop the audio
 * @return true on success, false on failure
 */
bool audio_output_set_loop(audio_instance_t* audio, bool loop);

/**
 * Check if an audio instance is currently playing
 * @param audio Pointer to the audio instance
 * @return true if playing, false if paused or stopped
 */
bool audio_output_is_playing(audio_instance_t* audio);

/**
 * Free resources associated with an audio instance
 * @param audio Pointer to the audio instance
 */
void audio_output_destroy(audio_instance_t* audio);

/**
 * Set the global audio volume (affects all channels)
 * @param volume Volume level (0.0 - 1.0)
 * @return true on success, false on failure
 */
bool audio_output_set_global_volume(float volume);

/**
 * Mute or unmute all audio
 * @param mute true to mute, false to unmute
 * @return true on success, false on failure
 */
bool audio_output_set_mute(bool mute);

/**
 * Stop all currently playing audio
 * @return true on success, false on failure
 */
bool audio_output_stop_all(void);

/**
 * Get number of audio channels supported by the system
 * @return Number of channels or 0 if audio not supported
 */
int audio_output_get_channel_count(void);

#endif /* AUDIO_OUTPUT_HANDLER_H */