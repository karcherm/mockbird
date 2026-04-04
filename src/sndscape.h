#include <cstdint>
#include "layer.h"

namespace Soundscape {

/// Version number, consisting of a major and minor version.
struct Version {
    std::uint8_t major;  ///< major verison.
    std::uint8_t minor;  ///< minor version.
};

/**
 * Information about the synthesizer setup.
 *
 * In case of a standard SoundScape card with a General MIDI ROM installed,
 * then number of melodic programs should always be 128. The number of patches
 * and waves default to 256 and 512, and should not be lower than the number
 * of patches and waves defined in the ROM. The number of drum instruments
 * is always 88.
 */
struct FWInfo {
    Version version;            ///< firmware version
    std::uint16_t melodic_cnt;  ///< number of melodic programs supported
    std::uint16_t patch_cnt;    ///< number of patches supported
    std::uint16_t wave_cnt;     ///< number of waves supported
    std::uint16_t drum_cnt;     ///< number of drum instruments supported
};

/**
 * A byte, tagged as "command" or "data" byte.
 *
 * This tagging is used to multiplex a "command" and a "data" communication
 * channel over the same physical port. Each of these channels is able to
 * transport 8-bit bytes. This is not the MIDI distinction between MIDI
 * command bytes (high bit set) and MIDI data bytes (high bit clear),
 * but an additional layer provided by the ODIE hardware.
 *
 * The Mockingbird-OTTO firmware can be configured into a mode in which the
 * secondary communication port transfers MIDI data from/to the internal
 * synthesizer on the "data" channel, while it transfers firmware commands
 * and replies on the "command" channel. This mode is typically not used
 * in DOS setups, because the MPU401-compatible mode is preferred in that
 * case.
 */
typedef struct {
    std::uint8_t data;  ///< payload byte
    bool is_command;    ///< channel indicator
} ss6850_byte;

/**
 * MC6850-compatible ODIE communication port.
 *
 * This class represents an ODIE communication port set to MS6850-compatible
 * mode. This is the only recommended mode for the secondary port, and its
 * command channel is used to interface the firmware.
 */
class SS6850Port {
public:
    /**
     * Initializes the port.
     *
     * The constructor writes a control value that takes the port out of
     * reset, after optionally setting it to reset first.
     *
     * @param base        I/O base address of the port. `0x332` for the
     *                    secondary port of a Soundscape in default config.
     * @param with_reset  If set, reset the port (not the firmware).
     * @throws HWMissingException  There obviously is no Sounscape port in
     *                             MC6850 mode at the given address.
     */
    SS6850Port(std::uint16_t base, bool with_reset);
    /**
     * Writes a byte to the port.
     *
     * @param data   Byte and target channel
     * @throws Timeout  The output buffer was stuck full for half a second.
     */
    void out(ss6850_byte data);
    /**
     * Reads a byte from the port (blocking).
     *
     * @return  Byte and source channel.
     * @throws Timeout  No byte arrived within half a second
     */
    ss6850_byte in();
    /**
     * Polls whether a byte is available.
     *
     * @return true if a byte is available.
     */
    bool in_avail();
    /**
     * Receives bytes from the port until no more bytes are available.
     */
    void drain();
private:
    void throw_hw_missing_exception();
    std::uint16_t base_port;
    // currently always zero; will change if IRQ support is added
    std::uint8_t control_val;
};

/**
 * MPU401-compatible ODIE communication port in UART mode
 *
 * This class represents an ODIE communication port set to MPU401-compatible
 * mode. This mode is typically used on the first port in DOS environments.
 */
class MPU401Port {
public:
    /**
     * Constructs a port.
     *
     * @param base        I/O base address of the port. `0x330` for the
     *                    primary port of a Soundscape in default config.
     */
    MPU401Port(std::uint16_t base);
    /**
     * Ensures the MPU401 port is set to operate in UART mode.
     *
     * This mode is used for "dumb" MIDI communication without any processing
     * by the MIDI Processing Unit, and the standard mode used in DOS
     * environments.
     *
     * This function should be called before the other methods, unless there
     * is prior knowledge that the port already is in UART mode.
     *
     * @throws HWMissingException  There obviously is no Sounscape port in
     *                             MPU401 mode at the given address.
     * @throws Timeout             Sending the reset or UART command took
     *                             more than half a second. Likely the hardware
     *                             is not present.
     */
    void ensure_uart_mode();
    /**
     * Sends a MIDI (OUT) byte.
     *
     * @param  data     MIDI byte to send.
     * @throws Timeout  The output buffer was stuck full for half a second.
     */
    void out(std::uint8_t data);
    /**
     * Polls whether a MIDI (IN) byte is available.
     *
     * @return true if a byte is available.
     */
    bool in_avail();
    /**
     * Reads a MIDI (IN) byte (blocking).
     *
     * @return the received MIDI byte
     * @throws Timeout  No byte was received within half a second.
     */
    std::uint8_t in();
private:
    void out_cmd(std::uint8_t command);
    void wait_out_rdy();
    std::uint16_t base_port;
};

/**
 * Definition of a melodic instrument.
 *
 * A melodic instruments consists of up to 4 layers, each of which may be
 * limited to apply only to a certain key or velocity range. This structure
 * contains a fixed number of 4 layers, some of which may be disabled by
 * clearing the `layers[i].exists` bit.
 *
 * This structure is a plain old data (POD) object, which (except for
 * endianess issues) is identical to the format used by the firmware.
 *
 * The textual representation of a instrument consists of up to 4 lines,
 * each describing a layer. If there are less than 4 lines, the remaining
 * layers do not exist. An alternative single-line representation uses
 * ` ; ` (a semicolon with at least one space on either side) instead
 * of line breaks.
 *
 * At the moment, layer parsing or formatting is not implemented in this
 * class.
 */
struct MelodicInstrument {
    /// The four layers
    InstrumentLayer layers[4];
};

/**
 * Interface to a Soundscape sound card running the Mockingbird-OTTO
 * firmware.
 *
 * This class uses the typical DOS configuration of running the first
 * port in MPU401 compatible mode that also works with most General MIDI
 * games. It will configure the firmware into that mode during
 * construction.
 *
 * This class does not implement any kind of interface to the AD1848
 * codec present on most SoundScape cards.
 *
 * Note that this is the "old" firmware distributed as `SNDSCAPE.COD`, not
 * the "new" firmware distributed as `SNDSCAPE.COx`. The new firmware does
 * not support synthesizer reprogramming. On the other hand, only the new
 * firmware supports the effects processor on the Soundscape Elite.
 *
 * The old firmware can be found at
 *  * https://discmaster.textfiles.com/browse/28230/MultimediaClassic.mdf/hardsoft/sndscape
 *    for Ensoniq S-2000 cards.
 *  * https://vogonsdrivers.com/getfile.php?fileid=1968 for Reveal SC-600
 *    cards.
 *  * no known location for S-1000 cards.
 *
 * The Spea/V7 MediaFX is a rebranded Reveal SC-600 if it has a TDA-8421
 * chip, or a rebranded Ensoniq S-2000 if it does not.
 * The X-Techsnd 001 is an S-1000 clone, the SJ-MS01 is another S-1000 clone.
 */
class Card {
public:
    /**
     * Interfaces to the card configured by the SNDSCAPE environment variable.
     *
     * This method parses `%SNDSCAPE%\SNDSCAPE.INI` to determine the base
     * port address. It throws an exception if the file does not exists or
     * the card does not respond.
     *
     * @throw AutoConfigError  Failed to determine the base port address from
     *                         the SoundScape configuration file
     * @throw HardwareNotPresent  The hardware is not responding at the port
     *                            indicated by the configuration file
     * @throw Timeout             While the hardware seems to be present, the
     *                            firmware does not respond as expected.
     * @throw UnexpectedEvent     The firmware sent an unexpected event byte
     *                            during initialization.
     * @throw FWError             The firmware reported an error during
     *                            initialization
     */
    Card();
    /**
     * Interfaces to the card at a specific port.
     *
     * @param port                The ODIE base I/O port.
     * @throw HardwareNotPresent  The hardware is not responding at the port
     *                            indicated by `port`.
     * @throw Timeout             While the hardware seems to be present, the
     *                            firmware does not respond as expected.
     * @throw UnexpectedEvent     The firmware sent an unexpected event byte
     *                            during initialization.
     * @throw FWError             The firmware reported an error during
     *                            initialization
     */
    Card(std::uint16_t port);

    // Firmware interaction
    /**
     * Get the firmware version.
     *
     * @return Firmware version, currently only version 1.21 is known to exist
     */
    Version get_fw_version();
    /**
     * Get Synthesizer info.
     *
     * This includes the firmware version as well as the table sizes for melodic
     * programs, patches, waveforms and drum instruments. Some of these parameters
     * can be changed using @ref reset_synth.
     *
     * @return Synthesizer info.
     */
    FWInfo get_fw_info();

    // Firmware interaction: Synth configuation
    /**
     * Clear software-loaded waves and synthesizer configuration.
     *
     * Resets the synthesizer. It automatically loads ROM samples if a sample
     * ROM is installed (which is the case on all known SoundScape cards).
     * It does not range check program, patch or wave numbers in the ROM,
     * so you must not specify values below the ones required by the ROM,
     * while means that `program_cnt` needs to be 128 unless your card is
     * modded. Values below 128 cause a firmware buffer overflow while it
     * loads the ROM samples, and values above 128 make no sense, because
     * there are only 128 MIDI programs. The Mockingbird-OTTO firmware does
     * not implement alternate banks to allow for more programs.
     *
     * While the firmware info includes the number of drum instruments, that
     * number is fixed and cannot be changed from 88 to anything else.
     *
     * The minimum number of patches or waveforms required for the default
     * general MIDI ROMs is not yet known.
     *
     * @param program_cnt  Size of the table for melodic programs (use 128).
     * @param patch_cnt    Size of the table for patches (firmware default
     *                      is 256).
     * @param wave_cnt     Size of the table for waveforms (firmware
     *                      default is 512).
     * @throw Timeout             The firmware did not respond in time.
     * @throw UnexpectedEvent     The firmware sent an unexpected event byte.
     * @throw FWError             The firmware reported an error.
     */

    void reset_synth(std::uint8_t program_cnt, std::uint16_t patch_cnt, std::uint16_t wave_cnt);
    /**
     * Gets the definition of one melodic instrument.
     *
     * @param program_nr          Number of the program (0..127)
     * @returns                   Instrument definition
     * @throw std::out_of_range   `program_nr` exceeds 127
     * @throw Timeout             The firmware did not respond in time.
     * @throw UnexpectedEvent     The firmware sent an unexpected event byte.
     * @throw FWError             The firmware reported an error.
     */
    MelodicInstrument get_melodic(std::uint8_t program_nr);
    /**
     * Sets the definition of one melodic instrument.
     *
     * @param program_nr          Number of the program (0..127)
     * @param inst                Instrument definition
     * @throw std::out_of_range   `program_nr` exceeds 127
     * @throw Timeout             The firmware did not respond in time.
     * @throw UnexpectedEvent     The firmware sent an unexpected event byte.
     * @throw FWError             The firmware reported an error.
     */
    void set_melodic(std::uint8_t program_nr, const MelodicInstrument& inst);

    // MIDI synth interface
    /**
     * Sends a byte to the MIDI synthesizer
     *
     * @param byte     MIDI byte to send.
     * @throw Timeout  The transmit buffer was stalled for more than half a
     *                 second.
     */
    void send_midi_byte(std::uint8_t byte);
private:
    // Common part of the constructors. Required because OpenWatcom 1.9
    // does not yet support delegating constructors.
    void _ctor_common();
    // Function to ensure the card is booted before initializing members
    // like host_port
    static std::uint16_t _ensure_booted(std::uint16_t port);

    // Internal firmware interaction
    void set_midi_port_mode(std::uint8_t mode);

    // Sends any byte to host interface
    void send_host_byte(std::uint8_t data);
    // Sends a 14-bit value to the host interface. Asserts if data is too big.
    void send_host_14bit(std::uint16_t data);
    // Waits for an ACK. Throws on unexpected replies.
    void expect_ack();
    // Receives a host byte. Drops "data" bytes. 
    std::uint8_t recv_host_raw_byte();
    // Receives a 7-bit data response. Throws on unexpected events (even ACK)
    std::uint8_t recv_host_7bit();
    // Receives a 14-bit data response combined from two 7-bit values
    std::uint16_t recv_host_14bit();

    std::uint16_t base_port;
    MPU401Port midi_port;
    SS6850Port host_port;
};
}
