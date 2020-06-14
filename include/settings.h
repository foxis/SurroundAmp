#if !defined(MY_SETTINGS_H)
#define MY_SETTINGS_H

#include <Arduino.h>

#define NUM_SR_CHANNELS 6
#define NUM_PRESETS 4
#define NUM_TONES 3
#define NUM_INPUTS 5
#define MIN_MASTER_VOLUME 0
#define MAX_MASTER_VOLUME 79
#define MIN_CHANNEL_TRIM 0
#define MAX_CHANNEL_TRIM 15
#define MIN_TONE -14
#define MAX_TONE 14

typedef uint8_t channel_t;
typedef int8_t tone_t;

typedef struct {
        uint8_t channel: 4;
        uint8_t es: 1; // Enhanced Surround 
        uint8_t mixed: 1;
} input_t;

typedef struct {
    input_t input;
    tone_t tone[NUM_TONES];
    channel_t channels[NUM_SR_CHANNELS];
    uint8_t flags;
} preset_t;

typedef struct {
    uint8_t fl: 1;
    uint8_t fr: 1;
    uint8_t ct: 1;
    uint8_t lfe: 1;
    uint8_t sl: 1;
    uint8_t sr: 1;
    uint8_t tone: 1;
    uint8_t all: 1;
} mute_t;

typedef struct {
    input_t input;
    mute_t mute;
    channel_t volume;
    tone_t tone[NUM_TONES];
    channel_t channels[NUM_SR_CHANNELS];
} master_t;

typedef struct {
    master_t master;
    uint8_t selected_preset;
    preset_t presets[NUM_PRESETS];
} settings_t;

settings_t settings = {
    .master = {
        .input={
            .channel=4,
            .es=0,
            .mixed=0,
        },
        .mute={
            .fl=0,
            .fr=0,
            .ct=0,
            .lfe=0,
            .sl=0,
            .sr=0,
            .tone=1,
            .all=0,
        },
        .volume=15,
        .tone={0, 0, 0},
        .channels={5, 5, 5, 5, 5, 5},
    },
    .selected_preset = 255,
    .presets={
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={5, 6, 5, 5, 5, 6},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={7, 0, 0},
            .channels={6, 5, 6, 5, 6, 5},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={5, 5, 5, 5, 5, 5},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={5, 5, 5, 5, 5, 5},
            .flags=0,
        },
    },
};

int read_settings(settings_t * settings) {

    return 0;
}

int write_settings(settings_t * settings) {
    return 0;
}

#endif
