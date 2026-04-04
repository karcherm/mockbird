#ifndef LAYER_H_INCLUDED_
#define LAYER_H_INCLUDED_
#include <cstdint>
#include <cstddef>

namespace Soundscape {
/**
 * One layer of a melodic instrument.
 *
 * This structure describes the binary format of the description of one
 * layer of an melodic instrument. A melodic instrument consists of up
 * to four layers, each of which can be limited to a certain velocity or
 * key range (but not both).
 *
 * Each layer describes a single patch to be played. Most synthesizer
 * parameters are specified on the patch level, the layer specification
 * adds extra modifiers, though. Furthermore, patches are monophonic, so
 * panning/stereo definition is fully specified at the layer layer.
 *
 * This class also implements a textual representation of the layer
 * definition. The syntax is quite strict, case sensitive (although it
 * needs to be completely lowercase, so a frontend might lowercase its
 * input to provide case-insignificance).
 *
 * The layer syntax consists of these elements:
 * 1. An optional condition specification, bracketed in `[` and `]`. The
 *    opening bracket is followed the keyword `vel` or `key` is expected,
 *    without a space inbetween. After the keyword, one or more spaces
 *    are required before a number, which indicates the minimum
 *    key or velocity this layer applies to. If the `]` follows directly,
 *    the layer applies only to a single key or velocity, otherwise, a
 *    `-` and the maximum key or velocity is required before the closing
 *    bracket (no spaces). If this optional specification is missing,
 *    this layer applies for any note.
 * 2. The patch number, followed by at least one space.
 * 3. The volume specification. It specifies an attenuation of the patch
 *    by 0dB to 63.5dB at a resolution of 0.5dB. As this is an
 *    attenuation, the specification is a negative number or zero, with
 *    the fractional part missing, being `.5` or being `.0`. This number
 *    may be followed by an optional boost specification spelled `+6`
 *    which sets a control bit that the patch is to be played 6dB louder.
 *    the volume specification ends in `db` (no capital `B`). There are
 *    no space characters in the entire volume specification.
 * 4. An optional tuning adjustment. It consists of the keyword `tune`
 *    followed by a number, optionally followed by an unit. A negative
 *    adjustment lowers the pitch, while a positive adjustment raises
 *    the pitch. The allowed units are `oct` for octaves, `st` for
 *    semitones or `ct` for cents (percent of a semitone). All of these
 *    scales specify a frequency multiplier, just as the raw scale does.
 *    The raw scale (no unit specified) is given in 2048th of an octave.
 *    So all these adjustments play the sample one octave above the
 *    indicated pitch: `1oct`, `12st`, `1200ct` or `2048`. If the tuning
 *    adjustment is missing, an adjustment of zero is used.
 * 5. An optional panning specification. The base panning is set by the
 *    MIDI panning control on the respective channel, and this panning
 *    is added to it. As with MIDI, the panning is given as 7-bit number.
 *    The adjustment is specified as a signed number between -64 and +63
 *    with 0 being "center". The resulting panning is clamped to that
 *    range as well. The panning specification consists of an offset
 *    optionally followed by a linear adjustment. If a linear adjustment
 *    is present, it is introduced with `+` or `-` after the offset,
 *    followed by a scale factor, which then must be followed by `*` and
 *    one of these 8 keywords:
 *     * `lfo1` - the output of the first LFO (-1..1)
 *     * `lfo2` - the output of the second LFO (-1..1)
 *     * `env1` - the output of the first envelope generator (0..1)
 *     * `ukey` - the key (unsigned), on a scale of 0..1
 *     * `skey` - the key (signed), on a scale of -1..1
 *     * `vel` - the velocity, on a scale of 0..1
 *     * `rnd` - a random value assigned at NOTE ON time
 *     * `mod` - the MIDI modulation wheel (-1..1)
 *
 *    Note that -1 actually means -1, while +1 is +127/128. In the case
 *    of `skey`, -1 is -127/128. As the modulation wheel only has 7 bit
 *    resolution, its range is -1 .. +63/64.
 *
 * A layer that has its `exist` bit clear is represented by the plain keyword `skip`
 *
 * This structure is a plain old data object, that is (except for endianess)
 * identical to the byte pattern used by the firmware.
 */
struct InstrumentLayer {
    /// number of the patch to play
    std::uint16_t patch : 10;
    /// fixed offset of panning adjustment (raw scale: -32 to +31)
    std::int16_t base_pan : 6;
    /// pitch adjustment in 2048th of an octave
    std::int16_t pitch;
    /// attenuation of 63.5dB (0) to 0dB (127) given in steps of 0.5dB
    std::uint8_t volume : 7;
    /// set if this layer is present
    std::uint8_t exists : 1;
    /// parameter used for linear panning adjustment
    std::uint8_t pan_mode : 3;
    /**
     * scale for linear panning adjustment.
     *
     * * -16 maps the parameter to the inverted full MIDI range
     * * 15 maps the parameter to the nearly the full MIDI range
     */
    std::int8_t pan_scale : 5;
    /// minimum (inclusive) of limited key or velocity range, 0 if no limit
    std::uint8_t min_val : 7;
    /// if set, limit on velocity, if clear, limit on key
    std::uint8_t dispatch_on_velocity : 1;
    /// maximim (inclusive) of limited key or velocity range, 127 if no limit
    std::uint8_t max_val : 7;
    /// add a fixed 6dB boost to the patch if set.
    std::uint8_t boost6db : 1;

    /// maximum length of patch description returned by @ref format.
    static const std::size_t maxlen = 60;
    /**
     * order of bytes to be sent or received from the big-endian firmware.
     *
     * As the first two of the 8 bytes are 16-bit quantities, those need to
     * be swapped, so this constant is
     *
     * ```
     * {1, 0, 3, 2, 4, 5, 6, 7}
     * ```
     */
    static const std::uint8_t endian_remap[];
    /// keywords for the 8 possible linear panning parameters
    static const char * const pan_keyword[];
    /**
     * parse a layer specification according to the specication given in the
     * class comment.
     *
     * @param  input  C string containing the textual representation
     * @return        Instrument layer
     * @throw         `std::runtime_error` or a derived class on invalid
     *                inputs. 
     */
    static InstrumentLayer parse(const char* input);
    /**
     * formats the specification as string.
     *
     * @param target  the buffer, which should be of size @ref maxlen.
     * @return        the buffer given by `target`.
     */
    char* format(char* target);
};
}
#endif
