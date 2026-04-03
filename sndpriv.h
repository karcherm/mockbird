// ODIE port offsets
#define PRIM_CHANNEL 0          // primary communication channel (0, 1)
#define SEC_CHANNEL 2           // secondary communication channel (2, 3)

#define PRIM_MPU_DATA 0         // MIDI data for primary port in MPU mode
#define PRIM_MPU_CMD 1          // MPU command for primary port
#define PRIM_MPU_STATUS 1       // MPU status bits for primary port

#define CFG_INDEX 4
#define CFG_DATA 5

// ODIE configuration registers
#define CFG_INDEX_MASTERCTL 9   // Host Master Control Register
#define MASTERCTL_SUBSYSCTL 0xC0// Mask for "master subsystem control" bits
#define SUBSYSCTL_RESET i         0x00
#define SUBSYSCTL_NORMALOPERATION 0x40
#define SUBSYSCTL_FWDOWNLOAD      0x80
#define SUBSYSCTL_FWBOOT          0xC0

// secondary port in MPU mode is not recommended by the data sheet,
// defines for SEC_MPU_DATA, SEC_MPU_CMD, SEC_MPU_STATUS omitted.
// MPU/6850 mode is controlled by firmware, and we don't expect any
// firmware to have the secondary port operating in MPU mode.
// Quote:
//  "When in MPU-401 emulation mode, RxRDY interrupts are ALWAYS enabled.
//   For maximum compatibility when operating in MPU-401 emulation mode,
//   the Host Interrupt Configuration register should be programmed to
//   provide a separate interrupt for the MIDI Emulation, otherwise all
//   other interrupts can be blocked by ill-behaved applications software.
//   Because of this, MPU-401 mode may not generally be usable on  the HOST
//   interface, but is provided there purely for the sake of symmetry."
// "HOST interface" refers to the secondary port. The idea behind this
// paragraph is the model of having a basic SoundScape driver that handles
// basic SoundScape functionality (like mixer and wave via synth), but
// hands out an emulated MIDI port to an application that interfaces directly
// to an MPU401-like interface. If an application causes the MPU401 to
// provide a byte (like an ACK to various commands), and doesn't read it,
// the MPU401 interrupt is stuck active. If the driver operates on the same
// IRQ, it is unable to receive low-level IRQs while the MPU401 rx byte is
// pending.


#define SNDSCAPE_EVT_ACK 0x80
#define SNDSCAPE_EVT_WAVEA 0x90
#define SNDSCAPE_EVT_WAVEB 0x91
#define SNDSCAPE_EVT_ERROR 0xFF

// Basic synth interface
#define SNDSCAPE_CMD_LOAD_WAVEFORM 0x80
#define SNDSCAPE_CMD_RESET_WAVEMEM 0x81
#define SNDSCAPE_CMD_SET_WAVEMAP 0x82
#define SNDSCAPE_CMD_ALIAS_WAVEFORM 0x83
#define SNDSCAPE_CMD_FREE_WAVEFORM 0x84
#define SNDSCAPE_CMD_GET_WAVEMEM 0x85
#define SNDSCAPE_CMD_SET_PATCH 0x86
#define SNDSCAPE_CMD_SET_MELODIC 0x87
#define SNDSCAPE_CMD_SET_MIXER 0x88
#define  MIXER_CHANNEL_MASTER_PLAYBACK 0       // S-1000 only
#define  MIXER_CHANNEL_MASTER_RECORD 1         // S-1000 only
#define  MIXER_CHANNEL_MICorLINE 2             // S-1000 only?
#define  MIXER_CHANNEL_LMC385CTL 3             // unknown, S-1000 only
#define  MIXER_CHANNEL_SYNTH 4
#define  MIXER_CHANNEL_WAVE_A 5
#define  MIXER_CHANNEL_WAVE_A_PAN 6
#define  MIXER_CHANNEL_WAVE_B 7
#define  MIXER_CHANNEL_WAVE_B_PAN 8
#define  MIXER_CHANNEL_OTTO_LEFT_PLAYBACK 9    // S-1000 only
#define  MIXER_CHANNEL_OTTO_RIGHT_PLAYBACK 10  // S-1000 only
#define  MIXER_CHANNEL_CD_LEFT_PLAYBACK 11     // S-1000 only
#define  MIXER_CHANNEL_CD_RIGHT_PLAYBACK 12    // S-1000 only
#define  MIXER_CHANNEL_LINE_LEFT_PLAYBACK 13   // S-1000 only
#define  MIXER_CHANNEL_LINE_RIGHT_PLAYBACK 14  // S-1000 only
#define  MIXER_CHANNEL_MIC_LEFT_PLAYBACK 15    // S-1000 only
#define  MIXER_CHANNEL_MIC_RIGHT_PLAYBACK 16   // S-1000 only
#define  MIXER_CHANNEL_OTTO_LEFT_RECORD 17     // S-1000 only
#define  MIXER_CHANNEL_OTTO_RIGHT_RECORD 18    // S-1000 only
#define  MIXER_CHANNEL_CD_LEFT_RECORD 19       // S-1000 only
#define  MIXER_CHANNEL_CD_RIGHT_RECORD 20      // S-1000 only
#define  MIXER_CHANNEL_LINE_LEFT_RECORD 21     // S-1000 only
#define  MIXER_CHANNEL_LINE_RIGHT_RECORD 22    // S-1000 only
#define  MIXER_CHANNEL_BASS 23                 // SC600 only
#define  MIXER_CHANNEL_TREBLE 24               // SC600 only
#define  MIXER_CHANNEL_STEREOFX 25             // SC600 only
#define  MIXER_CHANNEL_MUTE 26                 // SC600 only
#define SNDSCAPE_CMD_GET_MIXER 0x89
#define SNDSCAPE_CMD_GET_POLYPHONY 0x8A

// basic wave interface
#define SNDSCAPE_CMD_START_WAVE 0x8B
#define SNDSCAPE_CMD_PAUSE_WAVE 0x8C
#define SNDSCAPE_CMD_STOP_WAVE 0x8D

// advanced wave interface
#define SNDSCAPE_CMD_WAVE_A_TAILSIZE 0x90
#define SNDSCAPE_CMD_WAVE_B_TAILSIZE 0x91
// diagnostics
#define SNDSCAPE_CMD_PEEK_OBP 0x92
#define SNDSCAPE_CMD_POKE_OBP 0x93
// effect processor support
#define SNDSCAPE_CMD_ASSIGN_WAVE 0x94

// advanced synth setup
#define SNDSCAPE_CMD_SET_MIDI_PORT_MODE 0x98
#define  MIDI_PORT_MODE_6850 0
#define  MIDI_PORT_MODE_MPU401 1
#define SNDSCAPE_CMD_GET_MIDI_PORT_MODE 0x99
#define SNDSCAPE_CMD_GET_MIDI_MASK 0x9A
#define SNDSCAPE_CMD_SET_MIDI_MASK 0x9B
#define SNDSCAPE_CMD_send_some_data 0x9C
#define SNDSCAPE_CMD_SET_MIDIIN_SYNTH 0x9D
#define SNDSCAPE_CMD_GET_MIDIIN_SYNTH 0x9E
#define SNDSCAPE_CMD_GET_INFO 0x9F
#define SNDSCAPE_CMD_GET_WAVEFORM_COUNT 0xA0
#define SNDSCAPE_CMD_SET_DRUM_CHANNEL 0xA1
#define SNDSCAPE_CMD_NO_DRUM_CHANNEL 0xA2
#define SNDSCAPE_CMD_GET_PATCH 0xA3
#define SNDSCAPE_CMD_GET_MELODIC 0xA4
#define SNDSCAPE_CMD_GET_DRUM_CHANNEL 0xA5
#define SNDSCAPE_CMD_SET_TUNING 0xA6
#define SNDSCAPE_CMD_GET_TUNING 0xA7
#define SNDSCAPE_CMD_EDIT_WAVE_INFO 0xA8
#define SNDSCAPE_CMD_GET_WAVE_INFO 0xA9
#define SNDSCAPE_CMD_GET_WAVEMAP 0xAA
#define SNDSCAPE_CMD_GET_ALIAS_TARGET 0xAB
#define SNDSCAPE_CMD_GET_WAVEFORM_INFO 0xAC
#define SNDSCAPE_CMD_DEFRAG 0xAD
#define SNDSCAPE_CMD_SET_DRUM 0xAE
#define SNDSCAPE_CMD_GET_DRUM 0xAF
#define SNDSCAPE_CMD_SET_something 0xB0  // S-1000 maybe?
#define SNDSCAPE_CMD_GET_something 0xB1  // always 0 on SC600 and newer
#define SNDSCAPE_CMD_SET_SC_COMPAT 0xB2
#define SNDSCAPE_CMD_GET_SC_COMPAT 0xB3
#define SNDSCAPE_CMD_SET_CHANNEL_PRIO 0xB4
#define SNDSCAPE_CMD_GET_CHANNEL_PRIO 0xB5

// advanced host interface
#define SNDSCAPE_CMD_SET_WAVE_RATE 0xB6
#define SNDSCAPE_CMD_GET_WAVE_RATE 0xB7
#define SNDSCAPE_CMD_SET_MIDI_CLOCK 0xB8
#define SNDSCAPE_CMD_GET_MIDI_CLOCK 0xB9
#define SNDSCAPE_CMD_CONTINUE_WAVE 0xBA
#define SNDSCAPE_CMD_GET_FW_TYPE 0xBB
#define  FWTYPE_MIDI 0
#define  FWTYPE_FM 1
#define SNDSCAPE_CMD_GET_LIBRARY_ID 0xBC
#define SNDSCAPE_CMD_SET_LIBRARY_ID 0xBD

