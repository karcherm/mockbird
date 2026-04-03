#include "sndscape.h"
#include "sndexcpt.h"
#include "sndpriv.h"
#include "util.h"

#include <cassert>
#include <time.h>
#include <conio.h>

using namespace Soundscape;

#define SS6850_CTL 0            // 6850 control port
#define SS6850_STS 0            // 6850 status port
#define SS6850_DATA 1           // 6850 data port

// Status bits of ODIE ports in "MC6850 emulation" mode
#define SS6850_STS_RXRDY 0x01  // like actual 6850 status
#define SS6850_STS_TXRDY 0x02  // like actual 6850 status
#define SS6850_STS_CMD   0x04  // ODIE specific
                               //  0 = RX is MIDI byte
                               //  1 = RX is firmware response
#define SS6850_STS_IRQ   0x80  // like actual 6850 status

// Control bits of ODIE port in "MC6850 emulation" mode
#define SS6850_CTL_RESET 0x03  // like on actual 6850
                               //  but 00/01/02 all just mean "no reset"
#define SS6850_CTL_DATA  0x04  // ODIE specific
                               //  0 = TX data is firmware command
                               //  1 = TX data is MIDI byte
#define SS6850_CTL_TXINT 0x20  // like on actual 6850
                               //  but 00/40/60 all just mean "no TXINT"
#define SS6850_CTL_RXINT 0x80  // like on actual 6850

SS6850Port::SS6850Port(std::uint16_t port, bool with_reset)
    : base_port(port),
      control_val(0)
{
    if (with_reset)
    {
        outp(base_port + SS6850_CTL, SS6850_CTL_RESET);
        // status register is always 0 during reset
        if (inp(base_port+SS6850_CTL) != 0)
            throw_hw_missing_exception();
    }
    // end reset (if any)
    outp(base_port + SS6850_CTL, control_val);

    // status register has a lot of known bits after reset:
    // RXRDY is actually unknown
    // TXRDY should be set, as resetting the port drops the buffered byte
    // CMD is actually unknown (keeps last state)
    // bits 3..6 are documented to be "always zero"
    // bit 7 (IRQ pending) is zero if no IRQ is enabled (made sure by
    //   writing 0 to the control port)
    if ((inp(base_port + SS6850_STS) &
            ~(SS6850_STS_CMD | SS6850_STS_RXRDY))
        != SS6850_STS_TXRDY)
        throw_hw_missing_exception();
}

void SS6850Port::out(ss6850_byte val)
{
    clock_t start = clock();
    while (!(inp(base_port + SS6850_STS) & SS6850_STS_TXRDY))
    {
        if ((clock() - start) > CLOCKS_PER_SEC/2)
            throw Timeout();
    }
    std::uint8_t cmdbit = val.is_command ? 0 : SS6850_CTL_DATA;
    outp(base_port + SS6850_CTL, control_val | cmdbit);
    outp(base_port + SS6850_DATA, val.data);
}

bool SS6850Port::in_avail()
{
    return (inp(base_port + SS6850_STS) & SS6850_STS_RXRDY) != 0;
}

ss6850_byte SS6850Port::in()
{
    clock_t start = clock();
    while (!in_avail())
    {
        if ((clock() - start) > CLOCKS_PER_SEC/2)
            throw Timeout();
    }
    bool is_command = (inp(base_port + SS6850_STS) & SS6850_STS_CMD) != 0;
    std::uint8_t data = (std::uint8_t)inp(base_port + SS6850_DATA);
    ss6850_byte result = { data, is_command };
    return result;
}

void SS6850Port::drain()
{
    int drain_cnt = 0;
    while (in_avail())
    {
        inp(base_port + SS6850_DATA);
        // bit stuck high: likely not interfacing with a SoundScape
        // board
        if (drain_cnt > 50)
            throw_hw_missing_exception();
        drain_cnt++;
    }
}

void SS6850Port::throw_hw_missing_exception()
{
    throw CardNotPresent(base_port & 0xFFF0);
}

#define MPU401_DATA 0
#define MPU401_STS 1
#define MPU401_STS_TXFULL 0x40
#define MPU401_STS_RXEMPTY 0x80
#define MPU401_CMD 1
#define MPU401_CMD_UART 0x3F
#define MPU401_CMD_RESET 0xFF
#define MPU401_REPLY_ACK 0xFE

MPU401Port::MPU401Port(std::uint16_t port)
        : base_port(port)
{
}

void MPU401Port::ensure_uart_mode()
{
    if ((inp(base_port + MPU401_STS) & 0x3F) != 0x3F)
        throw CardNotPresent(base_port);
    // Two resets ensure we see an ACK (just one, because reset clears
    // any earlier bytes)
    out_cmd(MPU401_CMD_RESET);
    out_cmd(MPU401_CMD_RESET);
    if (in() != MPU401_REPLY_ACK)
        throw CardNotPresent(base_port);
    out_cmd(MPU401_CMD_UART);
    if (in() != MPU401_REPLY_ACK)
        throw CardNotPresent(base_port);
}

bool MPU401Port::in_avail()
{
    return (inp(base_port + MPU401_STS) & MPU401_STS_RXEMPTY) == 0;
}

std::uint8_t MPU401Port::in()
{
    clock_t start = clock();
    while (!in_avail())
    {
        if ((clock() - start) > CLOCKS_PER_SEC/2)
            throw Timeout();
    }
    return (std::uint8_t)inp(base_port + MPU401_DATA);
}

void MPU401Port::wait_out_rdy()
{
    clock_t start = clock();
    while (inp(base_port + MPU401_STS) & MPU401_STS_TXFULL)
    {
        if ((clock() - start) > CLOCKS_PER_SEC/2)
            throw Timeout();
    }
}

void MPU401Port::out(std::uint8_t data)
{
    wait_out_rdy();
    outp(base_port + MPU401_DATA, data);
}

void MPU401Port::out_cmd(std::uint8_t command)
{
    wait_out_rdy();
    outp(base_port + MPU401_CMD, command);
}

static std::uint16_t Card::_ensure_booted(std::uint16_t base_port)
{
    // verify that the firmware is booted
    outp(base_port + CFG_INDEX, CFG_INDEX_MASTERCTL);
    if ((inp(base_port + CFG_INDEX) & 0x0F) != CFG_INDEX_MASTERCTL)
        throw CardNotPresent(base_port);
    if ((inp(base_port + CFG_DATA) & MASTERCTL_SUBSYSCTL) !=
            SUBSYSCTL_NORMALOPERATION)
        throw CardNotInitialized();
    return base_port;
}

Card::Card(std::uint16_t port)
    : base_port(port),
      midi_port(port),
      host_port(_ensure_booted(base_port) + SEC_CHANNEL, true)
{
    _ctor_common();
}

void Card::_ctor_common()
{
    // Resetting the port clears one byte. If multiple bytes are
    // pending, multiple resets or dummy reads are required to clean up
    host_port.drain();

    // The "driver 1.2" firmware returns a single 00 byte to this command.
    // The "driver 1.0" firmware (the Mockingbird-OTTO one) returns firmware
    // info, starting with the major version (expect 1). The full firmware
    // info is 10 bytes long, so this code drains the remaining 9 bytes
    // after verifying that the correct firmware generation is active.
    send_host_byte(SNDSCAPE_CMD_GET_INFO);
    std::uint8_t response = recv_host_raw_byte();
    if (response == 0)
    {
        throw NewFirmware();
    }
    for (int i = 0; i < 9; i++)
        recv_host_7bit();

    set_midi_port_mode(MIDI_PORT_MODE_MPU401);
    midi_port.ensure_uart_mode();
}

Version Card::get_fw_version()
{
    return get_fw_info().version;
}

FWInfo Card::get_fw_info()
{
    FWInfo result;
    send_host_byte(SNDSCAPE_CMD_GET_INFO);
    result.version.major = recv_host_7bit();
    result.version.minor = recv_host_7bit();
    result.melodic_cnt = recv_host_14bit();
    result.patch_cnt = recv_host_14bit();
    result.wave_cnt = recv_host_14bit();
    result.drum_cnt = recv_host_14bit();
    return result;
}

void Card::reset_synth(std::uint8_t program_cnt, std::uint16_t patch_cnt, std::uint16_t wave_cnt)
{
    send_host_byte(SNDSCAPE_CMD_RESET_WAVEMEM);
    send_host_14bit(program_cnt);
    send_host_14bit(patch_cnt);
    send_host_14bit(wave_cnt);
    expect_ack();
}

void Card::set_melodic(std::uint8_t program, const MelodicInstrument& inst)
{
    send_host_byte(SNDSCAPE_CMD_SET_MELODIC);
    send_host_byte(program);
    for (size_t layer = 0; layer < ARRAYSIZE(inst.layers); layer++) {
        const char* read_ptr = (const char*)&inst.layers[layer];
        for (size_t i = 0; i < sizeof(inst.layers[0]); i++) {
            size_t sourceidx = InstrumentLayer::endian_remap[i];
            send_host_14bit(read_ptr[sourceidx]);
        }
    }
    expect_ack();
}

MelodicInstrument Card::get_melodic(std::uint8_t program)
{
    if (program & 0x80)
        throw std::out_of_range("Program number exceeding 127");
    send_host_byte(SNDSCAPE_CMD_GET_MELODIC);
    send_host_byte(program);
    MelodicInstrument result;
    for (size_t layer = 0; layer < ARRAYSIZE(result.layers); layer++) {
        char* write_ptr = (char*)&result.layers[layer];
        for (size_t i = 0; i < sizeof(result.layers[0]); i++)
        {
            size_t destidx = InstrumentLayer::endian_remap[i];
            write_ptr[destidx] = (unsigned char)recv_host_14bit();
        }
    }
    return result;
}

void Card::send_midi_byte(std::uint8_t data)
{
    midi_port.out(data);
}

void Card::set_midi_port_mode(std::uint8_t newmode)
{
    send_host_byte(SNDSCAPE_CMD_SET_MIDI_PORT_MODE);
    send_host_byte(newmode);
    expect_ack();
}

void Card::send_host_byte(std::uint8_t val)
{
    ss6850_byte b = { val, true };
    host_port.out(b);
}

void Card::send_host_14bit(std::uint16_t val)
{
    assert(val < 0x4000);
    send_host_byte((std::uint8_t)(val & 0x7F));
    send_host_byte((std::uint8_t)(val >> 7));
}

uint8_t Card::recv_host_raw_byte()
{
    while (true)
    {
        ss6850_byte b = host_port.in();
        if (b.is_command)
        {
            return b.data;
        }
        // we don't expect data bytes on the secondary port, as the
        // card (will be) configured in MPU401 mode which doesn't send
        // data bytes there, so just drop them.
    }
}

uint8_t Card::recv_host_7bit()
{
    while (true)
    {
        std::uint8_t input = recv_host_raw_byte();
        if (input & 0x80)
        {
            if (input == SNDSCAPE_EVT_ERROR)
                throw FwError(recv_host_raw_byte());
            if (input == SNDSCAPE_EVT_WAVEA || input == SNDSCAPE_EVT_WAVEB)
                continue;
            throw UnexpectedEvent(input);
        }
        return input;
    }
}

void Card::expect_ack()
{
    while (true)
    {
        std::uint8_t input = recv_host_raw_byte();
        if (input == SNDSCAPE_EVT_WAVEA || input == SNDSCAPE_EVT_WAVEB)
            continue;
        if (input != SNDSCAPE_EVT_ACK)
            throw UnexpectedEvent(input);
        break;
    }
}

uint16_t Card::recv_host_14bit()
{
    std::uint16_t lowpart = recv_host_7bit();
    return lowpart | ((std::uint16_t)recv_host_7bit() << 7);
}
