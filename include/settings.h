#if !defined(MY_SETTINGS_H)
#define MY_SETTINGS_H

#include <Arduino.h>

#define DISPLAY_BL_MIN 4
#define DISPLAY_BL_SLEEP 2
#define DISPLAY_BL_MAX 100
#define DISPLAY_BL_DEFAULT 30
#define DISPLAY_BL_TIMEOUT 30000L
#define DISPLAY_BL_OFF_TIMEOUT 600000L

#define SETTINGS_MAGIC 0xDBAE
#define SETTINGS_ADDR 0x800FF00
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
    input_t input;                  // 1 byte
    tone_t tone[NUM_TONES];         // 3 bytes
    channel_t channels[NUM_SR_CHANNELS];    // 6 bytes
    uint8_t flags;                  // 1 byte
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
    input_t input;                  // 1 byte
    mute_t mute;                    // 1 byte
    channel_t volume;               // 1 byte
    tone_t tone[NUM_TONES];         // 3 bytes
    channel_t channels[NUM_SR_CHANNELS];    // 6 bytes
} master_t;

typedef struct {
    uint16_t magic;                 // 2 bytes
    master_t master;                // 12 bytes
    uint8_t selected_preset;        // 1 byte
    preset_t presets[NUM_PRESETS];  // 4 * 11 bytes
    uint8_t padding;                
} settings_t;                       // 15 + 44 + 1 = 60 bytes or 30 words


settings_t settings = {
    .magic=SETTINGS_MAGIC,
    .master={
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
        .volume=45,
        .tone={0, 0, 0},
        .channels={4, 6, 4, 4, 4, 7},
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
            .channels={4, 6, 5, 5, 5, 7},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={4, 4, 4, 4, 4, 4},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={4, 6, 8, 6, 4, 6},
            .flags=0,
        },
        {
            .input={
                .channel=4,
                .es=0,
                .mixed=0,
            },
            .tone={0, 0, 0},
            .channels={7, 5, 8, 6, 6, 4},
            .flags=0,
        },
    },
};

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)

int  writeSector(uint32_t Address,void * values, uint16_t size)
{              
    uint16_t *AddressPtr;
    uint16_t *valuePtr;
    AddressPtr = (uint16_t *)Address;
    valuePtr=(uint16_t *)values;
    size = size / 2;  // incoming value is expressed in bytes, not 16 bit words
    while(size) {        
        // unlock the flash 
        // Key 1 : 0x45670123
        // Key 2 : 0xCDEF89AB
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
        FLASH->CR &= ~BIT1; // ensure PER is low
        FLASH->CR |= BIT0;  // set the PG bit        
        *(AddressPtr) = *(valuePtr);
        while(FLASH->SR & BIT0); // wait while busy
        if (FLASH->SR & BIT2)
            return -1; // flash not erased to begin with
        if (FLASH->SR & BIT4)
            return -2; // write protect error
        AddressPtr++;
        valuePtr++;
        size--;
    }    
    return 0;    
}

void eraseSector(uint32_t SectorStartAddress)
{
    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
    FLASH->CR &= ~BIT0;  // Ensure PG bit is low
    FLASH->CR |= BIT1; // set the PER bit
    FLASH->AR = SectorStartAddress;
    FLASH->CR |= BIT6; // set the start bit 
    while(FLASH->SR & BIT0); // wait while busy
}

void readSector(uint32_t SectorStartAddress, void * values, uint16_t size)
{
    uint16_t *AddressPtr;
    uint16_t *valuePtr;
    AddressPtr = (uint16_t *)SectorStartAddress;
    valuePtr=(uint16_t *)values;
    size = size/2; // incoming value is expressed in bytes, not 16 bit words
    while(size)
    {
        *((uint16_t *)valuePtr)=*((uint16_t *)AddressPtr);
        valuePtr++;
        AddressPtr++;
        size--;
    }
}

int write_settings(settings_t * settings) {
    noInterrupts();
    eraseSector(SETTINGS_ADDR);
    writeSector(SETTINGS_ADDR, settings, sizeof(*settings));
    interrupts();
    return 0;
}

int read_settings(settings_t * _settings) {
    settings_t tmp;
    readSector(SETTINGS_ADDR, &tmp, sizeof(tmp));
    if (tmp.magic != SETTINGS_MAGIC) {
        write_settings(&settings);
        read_settings(_settings);
        return 0;
    }
    memcpy(_settings, &tmp, sizeof(tmp));
    return 0;
}



#endif
