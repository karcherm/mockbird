#include "layer.h"
#include "util.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <stdexcept>

using namespace Soundscape;

const char* const InstrumentLayer::pan_keyword[] = {
    "lfo1", "lfo2", "env1", "ukey",
    "skey", "vol", "rnd", "mod"
};
const std::uint8_t InstrumentLayer::endian_remap[] = {
    1, 0, 3, 2, 4, 5, 6, 7
};
char* InstrumentLayer::format(char* target)
{
    char* original_target = target;
    if (!exists) {
        std::strcpy(target, "skip");
        return original_target;
    }
    std::size_t n;
    if (min_val != 0 || max_val != 127) {
        n = std::sprintf(target, "[%s %d-%d] ",
            dispatch_on_volume ? "vol" : "key", min_val, max_val);
        target += n;
    }
    n = std::sprintf(target, "%d -%d%s%sdb", patch,
        63 - (volume / 2), (volume&1) ? "" : ".5", boost6db ? "+6" : "");
    target += n;

    if (pitch != 0) {
        if ((pitch & 2047) == 0) {
            n = std::sprintf(target, " tune %doct", pitch / 2048);
        } else {
            int cents = rescale(pitch, 1200, 2048);
            if (rescale(cents, 2048, 1200) == pitch) {
                if (cents % 100 == 0)
                    n = std::sprintf(target, " tune %dst", cents / 100);
                else
                    n = std::sprintf(target, " tune %dct", cents);
            } else {
                n = std::sprintf(target, " tune %d", pitch);
            }
        }
        target += n;
    }

    if (base_pan != 0 || pan_scale != 0) {
        n = std::sprintf(target, " pan %d", base_pan * 2);
        target += n;
        if (pan_scale != 0) {
            std::sprintf(target, "%+d*%s",
                pan_scale * 4, pan_keyword[pan_mode]);
        }
    }
    return original_target;
}

InstrumentLayer InstrumentLayer::parse(const char* input)
{
    InstrumentLayer layer;
    if (std::strcmp(input, "skip") == 0) {
        layer.exists = 0;
    } else {
        layer.exists = 1;
        char* endptr;
        if (*input == '[')
        {
            input++;
            if (std::strncmp(input, "key", 3) == 0)
                layer.dispatch_on_volume = false;
            else if (std::strncmp(input, "vol", 3) == 0)
                layer.dispatch_on_volume = true;
            else
                throw std::runtime_error("layer condition must be key or vol");
            input = skip_spaces(input + 3);
            unsigned long min = std::strtoul(input, &endptr, 10);
            if (min > 127)
                throw std::overflow_error("min key or vol above 127");
            unsigned long max = min;
            if (*endptr == '-') {
                max = std::strtoul(endptr + 1, &endptr, 10);
                if (max > 127)
                    throw std::overflow_error("max key or vol above 127");
                if (max < min)
                    throw std::runtime_error("range max below range min");
                input = endptr;
            }
            if (*input != ']')
                throw std::runtime_error("expecting ] at end of condition");
            layer.min_val = (std::uint8_t)min;
            layer.max_val = (std::uint8_t)max;
            input = skip_spaces(input + 1);
        } else {
            layer.min_val = 0;
            layer.dispatch_on_volume = false;
            layer.max_val = 127;
        }
        unsigned long patchnr = std::strtoul(input, &endptr, 10);
        if (patchnr > 1023)
            throw std::overflow_error("Patch number above 1023");
        if (*endptr != ' ')
            throw std::runtime_error("Patch number not terminated with space");
        layer.patch = (std::uint16_t)patchnr;
        layer.base_pan = 0;  // hoping the compiler can optimize patch/base_pan
        input = skip_spaces(endptr);
    
        long volume = std::strtol(input, &endptr, 10);
        if (volume < -63)
            throw std::underflow_error("Volume below -63.5dB");
        if (volume > 0)
            throw std::overflow_error("Volume above 0dB");
        std::uint8_t volume_code = (std::uint8_t)((volume+63) * 2);
        input = endptr;
        if (*input == '.')  // ".5"
        {
            if (input[1] == '0')
                volume_code++;
            else if (input[1] != '5')
                throw std::runtime_error("Volume fraction present but not .0 or .5");
            input += 2;
        } else
            volume_code++;
        layer.volume = volume_code;
        if (*input == '+') // "+6"
        {
            if (input[1] == '6')
                layer.boost6db = 1;
            else
                throw std::runtime_error("Volume boost specification must be +6");
            input += 2;
        } else
        layer.boost6db = 0;
        if (input[0] != 'd' || input[1] != 'b')
            throw std::runtime_error("Volume specification must end in db");
        input = skip_spaces(input + 2);

        if (std::strncmp(input, "tune", 4) == 0) {
            input = skip_spaces(input + 4);
            long amount = std::strtol(input, &endptr, 10);
            if (endptr == input)
                throw std::runtime_error("Expected numeric tuning amount");
            input = skip_spaces(endptr);
            if (std::strncmp(input, "oct", 3) == 0) {
                // octaves
                amount *= 2048;
                input = skip_spaces(input + 3);
            } else if (std::strncmp(input, "st", 2) == 0) {
                // semitones
                amount = rescale((int)amount, 2048, 12);
                input = skip_spaces(input + 2);
            } else if (std::strncmp(input, "ct", 2) == 0) {
                amount = rescale((int)amount, 2048, 1200);
                input = skip_spaces(input + 2);
            }
            if (amount > 32767)
                throw std::overflow_error("Tuning exceeds +16 octaves");
            if (amount < -32768)
                throw std::underflow_error("Tuning exceeds -16 octaves");
            layer.pitch = (int16_t)amount;
        } else
            layer.pitch = 0;

        layer.pan_mode = 0;
        layer.pan_scale = 0;
        if (std::strncmp(input, "pan", 3) == 0) {
            long amount = std::strtol(input+3, &endptr, 10);
            if (endptr == input)
                throw std::runtime_error("Expected numeric base panning");
            if (amount < -64)
                throw std::underflow_error("Base panning exceeds hard left");
            if (amount > 63)
                throw std::overflow_error("Base panning exceeds hard right");
            layer.base_pan = (std::uint8_t)amount / 2;
            input = endptr;
            if (*input == '+' || *input == '-')
            {
                long scale = std::strtol(input, &endptr, 10);
                if (scale < -64)
                    throw std::underflow_error("panning scale below -64");
                if (scale > 63)
                    throw std::overflow_error("panning scale above +63");
                if (*endptr != '*')
                    throw std::runtime_error("panning scale must be followed by *");
                input = endptr + 1;
                for (size_t i = 0;; i++) {
                    if (i == ARRAYSIZE(pan_keyword))
                        throw std::runtime_error("bad panning parameter");
                    const char* kw = pan_keyword[i];
                    size_t kwlen = std::strlen(kw);
                    if (std::strncmp(input, kw, kwlen) == 0) {
                        layer.pan_mode = (std::uint8_t)i;
                        layer.pan_scale = (std::int8_t)(scale/4);
                        input = skip_spaces(input + kwlen);
                        break;
                    }
                }
            } else if (*input == ' ' || *input == '\0') {
                input = skip_spaces(input);
            } else
                throw std::runtime_error("unexpected character after base panning");
        }
        if (*input != '\0')
            throw std::runtime_error("extra input at end of layer definition");
    }
    return layer;
}

