#ifndef MOKYOMIDI_CONSTANTS
#define MOKYOMIDI_CONSTANTS

#include <stdint.h>

/* all midi files should begin with "MThd" */
#define mm_requiredHeaderID ((uint32_t)0x4d546864)

/* all midi files have a header size of 6 */
#define mm_minimumHeaderSize ((uint32_t)6)

/* file format definitions */
#define mm_formatSingleTrack ((uint16_t)0x0000)
#define mm_formatMultiTrackSync ((uint16_t)0x0001)
#define mm_formatMultiTrackAsync ((uint16_t)0x0002)
	
/* all midi tracks should begin with "MTrk" */
#define mm_requiredTrackHeaderID ((uint32_t)0x4d54726b)
	
/* status codes */
#define mm_statusMask ((uint8_t)0xF0)
/* note: midi event last 4 bits are channel number */
#define mm_noteOff ((uint8_t)0x80)
#define mm_noteOn ((uint8_t)0x90)
#define mm_polyphonicKeyPressure ((uint8_t)0xA0)
#define mm_controlChange ((uint8_t)0xB0)
#define mm_programChange ((uint8_t)0xC0)
#define mm_channelPressure ((uint8_t)0xD0)
#define mm_pitchBendChange ((uint8_t)0xE0)
#define mm_metaEventOrSysex ((uint8_t)0xF0)
/* meta events should not be sent to the synth */
#define mm_metaEvent ((uint8_t)0xFF)
/* 
 * system exclusive events should be sent to the synth 
 * including the byte F0
 */
#define mm_sysexStart ((uint8_t)0xF0)
/* 
 * used to end sysex, and also starts continuation
 * events and escape sequences
 */
#define mm_sysexEnd ((uint8_t)0xF7)
	
/* meta events have a secondary status code */
#define mm_metaSequenceNumber ((uint8_t)0x00)
#define mm_metaText ((uint8_t)0x01)
#define mm_metaCopyright ((uint8_t)0x02)
#define mm_metaSequenceOrTrackName ((uint8_t)0x03)
#define mm_metaInstrumentName ((uint8_t)0x04)
#define mm_metaLyric ((uint8_t)0x05)
#define mm_metaMarker ((uint8_t)0x06)
#define mm_metaCuePoint ((uint8_t)0x07)
#define mm_metaProgramName ((uint8_t)0x08)
#define mm_metaDeviceName ((uint8_t)0x09)
#define mm_metaMidiChannelPrefix ((uint8_t)0x20)
#define mm_metaMidiPort ((uint8_t)0x21)
#define mm_metaEndOfTrack ((uint8_t)0x2F)
#define mm_metaTempo ((uint8_t)0x51)
#define mm_metaSmpteOffset ((uint8_t)0x54)
#define mm_metaTimeSignature ((uint8_t)0x58)
#define mm_metaKeySignature ((uint8_t)0x59)
#define mm_metaSequencerSpecific ((uint8_t)0x7F)
	
/* controller codes */
#define mm_bankSelectMSB ((uint8_t)0)
#define mm_modulationWheelMSB ((uint8_t)1)
#define mm_breathControllerMSB ((uint8_t)2)
/* undefined 3 */
#define mm_footPedalMSB ((uint8_t)4)
#define mm_portamentoTimeMSB ((uint8_t)5)
#define mm_dataEntryMSB ((uint8_t)6)
#define mm_channelVolumeMSB ((uint8_t)7)
#define mm_mainVolumeMSB mm_channelVolumeMSB
#define mm_balanceMSB ((uint8_t)8)
/* undefined 9 */
#define mm_panMSB ((uint8_t)10)
#define mm_expressionMSB ((uint8_t)11)
#define mm_effectControl1MSB ((uint8_t)12)
#define mm_effectControl2MSB ((uint8_t)13)
/* undefined 14, 15 */
#define mm_generalPurposeController1MSB ((uint8_t)16)
#define mm_generalPurposeController2MSB ((uint8_t)17)
#define mm_generalPurposeController3MSB ((uint8_t)18)
#define mm_generalPurposeController4MSB ((uint8_t)19)
/* undefined 20-31 */
#define mm_bankSelectLSB ((uint8_t)32)
#define mm_modulationWheelLSB ((uint8_t)33)
#define mm_breathControllerLSB ((uint8_t)34)
/* undefined 35 */
#define mm_footPedalLSB ((uint8_t)36)
#define mm_portamentoTimeLSB ((uint8_t)37)
#define mm_dataEntryLSB ((uint8_t)38)
#define mm_channelVolumeLSB ((uint8_t)39)
#define mm_mainVolumeLSB mm_channelVolumeLSB
#define mm_balanceLSB ((uint8_t)40)
/* undefined 41 */
#define mm_panLSB ((uint8_t)42)
#define mm_expressionLSB ((uint8_t)43)
#define mm_effectControl1LSB ((uint8_t)44)
#define mm_effectControl2LSB ((uint8_t)45)
/* undefined 46, 47 */
#define mm_generalPurposeController1LSB ((uint8_t)48)
#define mm_generalPurposeController2LSB ((uint8_t)49)
#define mm_generalPurposeController3LSB ((uint8_t)50)
#define mm_generalPurposeController4LSB ((uint8_t)51)
/* undefined 52-63 */
#define mm_sustainSwitch ((uint8_t)64)
#define mm_portamentoSwitch ((uint8_t)65)
#define mm_sostenutoSwitch ((uint8_t)66)
#define mm_softPedalSwitch ((uint8_t)67)
#define mm_legatoSwitch ((uint8_t)68)
#define mm_hold2Switch ((uint8_t)69)
#define mm_soundController1 ((uint8_t)70)
#define mm_soundVariation mm_soundController1
#define mm_soundController2 ((uint8_t)71)
#define mm_soundTimbre mm_soundController2
#define mm_soundController3 ((uint8_t)72)
#define mm_releaseTime mm_soundController3
#define mm_soundController4 ((uint8_t)73)
#define mm_attackTime mm_soundController4
#define mm_soundController5 ((uint8_t)74)
#define mm_brightness mm_soundController5
#define mm_soundController6 ((uint8_t)75)
#define mm_decayTime mm_soundController6
#define mm_soundController7 ((uint8_t)76)
#define mm_vibratoRate mm_soundController7
#define mm_soundController8 ((uint8_t)77)
#define mm_vibratoDepth mm_soundController8
#define mm_soundController9 ((uint8_t)78)
#define mm_vibratoDelay mm_soundController9
#define mm_soundController0 ((uint8_t)79)
#define mm_generalPurposeController5 ((uint8_t)80)
#define mm_generalPurposeController6 ((uint8_t)81)
#define mm_generalPurposeController7 ((uint8_t)82)
#define mm_generalPurposeController8 ((uint8_t)83)
#define mm_portamentoControl ((uint8_t)84)
/* undefined 85-87 */
#define mm_highResolutionVelocityPrefix ((uint8_t)88)
/* undefined 89, 90 */
#define mm_effects1Depth ((uint8_t)91) 
#define mm_reverb mm_effects1Depth
#define mm_effects2Depth ((uint8_t)92) 
#define mm_tremelo mm_effects2Depth
#define mm_effects3Depth ((uint8_t)93) 
#define mm_chorus mm_effects3Depth
#define mm_effects4Depth ((uint8_t)94) 
#define mm_detune mm_effects4Depth
#define mm_delay mm_effects4Depth
#define mm_effects5Depth ((uint8_t)95) 
#define mm_phaser mm_effects5Depth
#define mm_dataIncrement ((uint8_t)96)
#define mm_dataDecrement ((uint8_t)97)
#define mm_nrpnLSB ((uint8_t)98)
#define mm_nrpnMSB ((uint8_t)99)
#define mm_rpnLSB ((uint8_t)100)
#define mm_rpnMSB ((uint8_t)101)
/* undefined 102-119 */
/* channel mode messages */
#define mm_allSoundOff ((uint8_t)120)
#define mm_resetAllControllers ((uint8_t)121)
#define mm_localControl ((uint8_t)122)
#define mm_allNotesOff ((uint8_t)123)
#define mm_omniModeOff ((uint8_t)124)
#define mm_omniModeOn ((uint8_t)125)
#define mm_monoModeOn ((uint8_t)126)
#define mm_polyModeOn ((uint8_t)127)

/* midi defaults to 120 BPM */
#define mm_defaultMicrosecondsPerBeat \
    ((uint32_t)500000)

/* Encodes the given fps in smpte format */
uint8_t smpteFpsEncode(uint32_t fps);

/* Decodes the given smpte code to fps */
uint32_t smpteFpsDecode(uint8_t code);

#endif