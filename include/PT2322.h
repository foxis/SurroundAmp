#if !defined(PT2322_H)
#define PT2322_H

#include <Wire.h>
#include "settings.h"

#define PT2322_ADDR 0b01000100
//#define PT2322_ADDR 0b10001000
#define PT2322_INIT_CODE 0b11000111
#define PT2322_CLEAR_CODE 0b11111111
#define PT2322_FL_TRIM 0b00010000
#define PT2322_FR_TRIM 0b00100000
#define PT2322_CT_TRIM 0b00110000
// Due to the wiring error these two are switched
#define PT2322_SL_TRIM 0b01010000
#define PT2322_SR_TRIM 0b01000000

#define PT2322_LFE_TRIM 0b01100000
#define PT2322_FUNCTION 0b01110000
#define PT2322_TONE_BASS 0b10010000
#define PT2322_TONE_MIDDLE 0b10100000
#define PT2322_TONE_TREBLE 0b10110000
#define PT2322_INPUT 0b11000000
#define PT2322_MASTER_1DB 0b11010000
#define PT2322_MASTER_10DB 0b11100000

#define PT2322_FUNCTION_MUTE 0b1000
#define PT2322_FUNCTION_3D 0b0100
#define PT2322_FUNCTION_TONE 0b0010


class PT2322 {
public:
    PT2322() {

    }

    void begin() {
        write8(PT2322_CLEAR_CODE);
        write8(PT2322_INIT_CODE);
    }

    void function(mute_t data) {
        uint8_t tmp = PT2322_FUNCTION_3D;
        tmp |= data.all ? PT2322_FUNCTION_MUTE : 0;
        tmp |= data.tone ? PT2322_FUNCTION_TONE : 0;
        write8(PT2322_FUNCTION | tmp);
    }

    void mute_all(bool val) {
        write8(PT2322_FUNCTION | ( val ? PT2322_FUNCTION_MUTE : 0 ));
    }

    void trim(channel_t * _channels) {
        channel_t channels[6];
        for (int i = 0; i < 6; i++)
            channels[i] = 15 - min(_channels[i], (uint8_t)15);
        Wire.beginTransmission(PT2322_ADDR);
        Wire.write(PT2322_FL_TRIM | channels[0]);
        Wire.write(PT2322_FR_TRIM | channels[1]);
        Wire.write(PT2322_CT_TRIM | channels[2]);
        Wire.write(PT2322_LFE_TRIM | channels[3]);
        Wire.write(PT2322_SL_TRIM | channels[4]);
        Wire.write(PT2322_SR_TRIM | channels[5]);
        Wire.endTransmission();
    }

    void tone(tone_t * _tone) {
        uint8_t tone[3];
        uint8_t map[] = {
            0b0000,  // -14
            0b0001,  // -12
            0b0010,  // -10
            0b0011,  // -8
            0b0100,  // -6
            0b0101,  // -4
            0b0110,  // -2
            0b0111,  // 0
            0b1110,  // +2 
            0b1101,  // +4
            0b1100,  // +6
            0b1011,  // +8
            0b1010,  // +10
            0b1001,  // +12
            0b1000,  // +14  
        };
        for (int i = 0; i < 3; i++) {
            tone_t tmp = (14 + max((tone_t)-14, min(_tone[i], (tone_t)14))) / 2;
            tone[i] = map[tmp];
        }
        Wire.beginTransmission(PT2322_ADDR);
        Wire.write(PT2322_TONE_BASS | tone[0]);
        Wire.write(PT2322_TONE_MIDDLE | tone[1]);
        Wire.write(PT2322_TONE_TREBLE | tone[2]);
        Wire.endTransmission();
    }

    void master(channel_t val) {
        val = 79 - min(val, (channel_t)79);
        Wire.beginTransmission(PT2322_ADDR);
        Wire.write(PT2322_MASTER_10DB | (val / 10));
        Wire.write(PT2322_MASTER_1DB | (val % 10));
        Wire.endTransmission();
    }

    void write8(uint8_t data) {
        Wire.beginTransmission(PT2322_ADDR);
        Wire.write(data);
        Wire.endTransmission();
    }
};

#endif
