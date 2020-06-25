#if !defined(PT2323_H)
#define PT2323_H

#include <Wire.h>
#include "settings.h"

#define PT2323_ADDR 74
//#define PT2323_ADDR 0b10010100

#define PT2323_INPUT_CHANNEL 0b11000000
#define PT2323_INPUT_MUTE_FL 0b11110000
#define PT2323_INPUT_MUTE_FR 0b11110010
#define PT2323_INPUT_MUTE_CT 0b11110100
#define PT2323_INPUT_MUTE_LFE 0b11110110
#define PT2323_INPUT_MUTE_SL 0b11111000
#define PT2323_INPUT_MUTE_SR 0b11111010
#define PT2323_INPUT_MUTE_ALL 0b11111110

#define PT2323_INPUT_ES 0b11010000
#define PT2323_INPUT_MIXED 0b10010000

#define PT2323_UNMUTE_ALL       0b11111110

class PT2323 {
public:
    PT2323() {

    }

    void begin() {
        Wire.beginTransmission(PT2323_ADDR);
        Wire.write(PT2323_UNMUTE_ALL);
        Wire.endTransmission();
    }

    void input(input_t data) {
        uint8_t map[] = {
            0b1011,  // Input Stereo Group 1
            0b1010,  // Input Stereo Group 2
            0b1001,  // Input Stereo Group 3
            0b1000,  // Input Stereo Group 4
            0b0111,  // 6ch input 
        };

        Wire.beginTransmission(PT2323_ADDR);
        Wire.write(PT2323_INPUT_CHANNEL | map[data.channel % 5]);
        Wire.write(PT2323_INPUT_ES | ((~data.es) & 0x01));
        Wire.write(PT2323_INPUT_MIXED | (data.mixed));
        Wire.endTransmission();
    }

    void mute(mute_t data) {
        Wire.beginTransmission(PT2323_ADDR);
        Wire.write(PT2323_INPUT_MUTE_FL | data.fl);
        Wire.write(PT2323_INPUT_MUTE_FR | data.fr);
        Wire.write(PT2323_INPUT_MUTE_CT | data.ct);
        Wire.write(PT2323_INPUT_MUTE_LFE | data.lfe);
        Wire.write(PT2323_INPUT_MUTE_SL | data.sl);
        Wire.write(PT2323_INPUT_MUTE_SR | data.sr);
        Wire.write(PT2323_INPUT_MUTE_ALL | data.all);
        Wire.endTransmission();
    }

    void mute_all(bool val) {
        Wire.beginTransmission(PT2323_ADDR);
        Wire.write(PT2323_INPUT_MUTE_ALL | (val ? 1 : 0));
        Wire.endTransmission();
    }
};

#endif
