#include "sndscape.h"
#include "util.h"
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <i86.h> // for delay

// MIDI commands
#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON 0x90
#define MIDI_NOTE_AFTERTOUCH 0xA0
#define MIDI_CTLCHANGE 0xB0
#define MIDI_PROGRAM 0xC0
#define MIDI_CHANNEL_AFTERTOUCH 0xD0
#define MIDI_PITCHBEND 0xE0

#define MIDI_KEY_MIDDLE_C 60

using namespace Soundscape;

void play_single_note_data(Card &sndscape,
        std::uint8_t key, std::uint8_t velocity, int duration)
{
    sndscape.send_midi_byte(key);
    sndscape.send_midi_byte(velocity);
    delay(duration);
    sndscape.send_midi_byte(key);
    sndscape.send_midi_byte(0);
}

void play_melody(Card &sndscape, std::uint8_t program)
{
    sndscape.send_midi_byte(MIDI_PROGRAM);
    sndscape.send_midi_byte(program);
    sndscape.send_midi_byte(MIDI_NOTEON);
    play_single_note_data(sndscape, MIDI_KEY_MIDDLE_C, 40, 300);
    play_single_note_data(sndscape, MIDI_KEY_MIDDLE_C + 4, 60, 300);
    play_single_note_data(sndscape, MIDI_KEY_MIDDLE_C + 7, 80, 300);
    play_single_note_data(sndscape, MIDI_KEY_MIDDLE_C + 12, 100, 300);
}

void dump_melodic(Card &sndscape, std::uint8_t program_nr)
{
    MelodicInstrument inst = sndscape.get_melodic(program_nr);
    int skips = 0;
    for (int j = 0; j < ARRAYSIZE(inst.layers); j++) {
        InstrumentLayer& layer = inst.layers[j];
        if (layer.exists) {
            while (skips) {
                std::puts("skip");
                skips--;
            }
            char buffer[InstrumentLayer::maxlen];
            std::puts(layer.format(buffer));
        } else {
            skips++;
        }
    }
}

void load_melodic(Card &sndscape, std::uint8_t program_nr, const char* lines[])
{
    MelodicInstrument inst;
    std::memset(&inst, 0, sizeof inst);
    for (size_t i = 0;; i++)
    {
        if (i == ARRAYSIZE(inst.layers))
            throw std::overflow_error("Too many layers in melodic instrument");
        if (lines[i] == NULL)
            break;
        inst.layers[i] = InstrumentLayer::parse(lines[i]);
    }
    sndscape.set_melodic(program_nr, inst);
}

void dump_all(Card &sndscape)
{
    FWInfo info = sndscape.get_fw_info();
    std::printf("# Found SoundScape firmware version %d.%d\n",
        info.version.major, info.version.minor);
    std::printf("# %d melodic programs, %d patches, %d waves\n",
        info.melodic_cnt, info.patch_cnt, info.wave_cnt);
    std::printf("# %d drum programs\n", info.drum_cnt);
    for (unsigned i = 0; i < info.melodic_cnt; i++) {
        std::printf("program %d\n", i);
        dump_melodic(sndscape, (std::uint8_t)i);
    }
}

void print_usage(std::FILE* dest)
{
    std::fputs(
"Mockingbird-OTTO control program v0.1\n"
"Supported parameters\n"
"  dump     - dump complete configuration of the synthesizer\n"
"  dumpprg [<min>] [<max>] - dump one or multiple melodic programs\n"
"    min defaults to 0, max defaults to 127 (if no min is given) or min\n"
"  loadprg <n> [<def>]    - load one melodic program\n"
"    load melodic program definition from stdin if def is missing\n"
"    otherwise load from command line, separate layers with \" ; \"\n"
"  play [<n>]             - plays a sample tune using program 0 or <n>\n"
"  reset [<pat>] [<wave>] - resets synth parameters\n"
"    resets the synthesizer with space for <pat> patches and <wave> waveforms\n"
"    If a sample ROM is installed, it will automatically be loaded. In that\n"
"    case, do not specify <pat> or <wave> lower than required by that ROM to\n"
"    prevent firmware data corruption\n"
"Program numbers are between 0 and 127, while MIDI often uses 1-128.\n",
    dest);
}

int main(int argc, char** argv)
{
    try {
        Card sndscape;
        if (argc < 2 || std::strcmp(argv[1], "dump") == 0)
            dump_all(sndscape);
        else if (std::strcmp(argv[1], "dumpprg") == 0) {
            int min = 0, max = 127;
            if (argc > 2) {
                std::sscanf(argv[2], "%d", &min);
                max = min;
            }
            if (argc > 3) {
                std::sscanf(argv[3], "%d", &max);
            }
            for (int i = min; i <= max; i++) {
                if (min != max)
                    std::printf("program %d\n", i);
                dump_melodic(sndscape, (std::uint8_t)i);
            }
        } else if (std::strcmp(argv[1], "loadprg") == 0) {
            if (argc < 3) {
                std::fputs("loadprg missing program number", stderr);
                return 1;
            }
            char defs[4][InstrumentLayer::maxlen];
            const char* linelist[5];
            int program = std::atoi(argv[2]);
            if (argv[3] == NULL) {
                size_t i;
                for (i = 0; i < 4; i++) {
                    std::fgets(defs[i], sizeof(defs[i]), stdin);
                    if (std::feof(stdin) || std::ferror(stdin) ||
                        defs[i][0] == '\n' || defs[i][0] == '\0')
                        break;
                    if (defs[i][0] == '#') {
                        i++;
                        continue;
                    }
                    size_t len = std::strlen(defs[i]);
                    if (defs[i][len-1] != '\n') {
                        std::fputs("overly long line", stderr);
                        return 1;
                    }
                    defs[i][len-1] = '\0';
                    linelist[i] = defs[i];
                }
                linelist[i] = NULL;
            } else {
                size_t line_nr = 0;
                defs[0][0] = '\0';
                for (int i = 3; i < argc; i++) {
                    if (std::strcmp(argv[i], ";") == 0) {
                        linelist[line_nr] = defs[line_nr];
                        if (line_nr == 3)
                            break;
                        line_nr++;
                        defs[line_nr][0] = '\0';
                    } else {
                        if (defs[line_nr][0] != '\0')
                            std::strcat(defs[line_nr], " ");
                        std::strcat(defs[line_nr], argv[i]);
                    }
                }
                linelist[line_nr] = defs[line_nr];
                linelist[line_nr+1] = NULL;
            }
            load_melodic(sndscape, (std::uint8_t)program, linelist);
        } else if (std::strcmp(argv[1], "play") == 0) {
            int program = 0;
            if (argc > 2)
                std::sscanf(argv[2], "%d", &program);
            play_melody(sndscape, (std::uint8_t)program);
        } else if (std::strcmp(argv[1], "reset") == 0) {
            std::uint16_t patches = 256;
            if (argc > 2)
                patches = std::atoi(argv[2]);
            std::uint16_t waves = 512;
            if (argc > 3)
                waves = std::atoi(argv[3]);
            // Fix programs to 128:
            // less than 128 programs is bad, because the firmware auto-loads
            // the GM ROM, which will overflow the table if less than 128
            // programes are allocated.
            // more than 128 programs makes no sense, because the firmware
            // does not support alternate banks, so only programs 0-127 are
            // accessible.
            sndscape.reset_synth(128, patches, waves);
	} else if (std::strcmp(argv[1], "help") == 0) {
	    print_usage(stdout);
        } else {
	    print_usage(stderr);
            return 1;
        }
    } catch(std::exception &e) {
        std::fputs(e.what(), stderr);
        return 1;
    }
    return 0;
}

