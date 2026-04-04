#ifndef LAYER_H_INCLUDED_
#define LAYER_H_INCLUDED_
#include <cstdint>
#include <cstddef>

namespace Soundscape {
struct InstrumentLayer {
    std::uint16_t patch : 10;
    std::int16_t base_pan : 6;
    std::int16_t pitch;
    std::uint8_t volume : 7;
    std::uint8_t exists : 1;
    std::uint8_t pan_mode : 3;
    std::int8_t pan_scale : 5;
    std::uint8_t min_val : 7;
    std::uint8_t dispatch_on_velocity : 1;
    std::uint8_t max_val : 7;
    std::uint8_t boost6db : 1;

    static const std::size_t maxlen = 60;
    static const std::uint8_t endian_remap[];
    static const char * const pan_keyword[];
    static InstrumentLayer parse(const char* input);
    char* format(char* target);
};
}
#endif
