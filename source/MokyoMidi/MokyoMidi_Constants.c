#include "MokyoMidi_Constants.h"

#include "PGUtil.h"

uint8_t smpteFpsEncode(uint32_t fps) {
	switch(fps) {
		case 24: return 0xE8;
		case 25: return 0xE7;
		case 29: return 0xE3;
		case 30: return 0xE2;
		default:
            pgError("Error invalid SMPTE fps");
            return 0;
	}
}
	
uint32_t smpteFpsDecode(uint8_t code) {
	switch(code) {
		case 0xE8: return 24;
		case 0xE7: return 25;
		case 0xE3: return 29;
		case 0xE2: return 30;
        default:
            pgError("Error invalid SMPTE code");
            return 0;
	}
}