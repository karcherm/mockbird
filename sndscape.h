#include <cstdint>
#include "layer.h"

namespace Soundscape {

struct Version {
    std::uint8_t major, minor;
};

struct FWInfo {
    Version version;
    std::uint16_t melodic_cnt, patch_cnt, wave_cnt;
    std::uint16_t drum_cnt;
};

typedef struct {
    std::uint8_t data;
    bool is_command;
} ss6850_byte;

class SS6850Port {
public:
    SS6850Port(std::uint16_t base, bool with_reset);
    void out(ss6850_byte data);
    ss6850_byte in();
    bool in_avail();
    void drain();
private:
    void throw_hw_missing_exception();
    std::uint16_t base_port;
    // currently always zero; will change if IRQ support is added
    std::uint8_t control_val;
};

class MPU401Port {
public:
    MPU401Port(std::uint16_t base);
    void ensure_uart_mode();
    void out(std::uint8_t data);
    bool in_avail();
    std::uint8_t in();
private:
    void out_cmd(std::uint8_t command);
    void wait_out_rdy();
    std::uint16_t base_port;
};

struct MelodicInstrument {
    InstrumentLayer layers[4];
};

class Card {
public:
    Card();
    Card(std::uint16_t port);

    // Firmware interaction
    Version get_fw_version();
    FWInfo get_fw_info();
    // Firmware interaction: Synth configuation
    void reset_synth(std::uint8_t program_cnt, std::uint16_t patch_cnt, std::uint16_t wave_cnt);
    MelodicInstrument get_melodic(std::uint8_t program_nr);
    void set_melodic(std::uint8_t program_nr, const MelodicInstrument& inst);

    // MIDI synth interface
    void send_midi_byte(std::uint8_t byte);
private:
    void _ctor_common();
    static std::uint16_t _ensure_booted(std::uint16_t port);

    // Internal firmware interaction
    void set_midi_port_mode(std::uint8_t mode);

    void send_host_byte(std::uint8_t data);
    void send_host_14bit(std::uint16_t data);
    void expect_ack();
    std::uint8_t recv_host_raw_byte();
    std::uint8_t recv_host_7bit();
    std::uint16_t recv_host_14bit();

    std::uint16_t base_port;
    MPU401Port midi_port;
    SS6850Port host_port;
};
}
