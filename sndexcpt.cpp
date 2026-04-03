#include "sndexcpt.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Soundscape;

Exception::Exception(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsprintf(message_buffer, format, args);
    va_end(args);
}

const char* Exception::what() const
{
    return message_buffer;
}

CardNotPresent::CardNotPresent(std::uint16_t port)
    : Exception("Soundscape not found at port %03x", port)
{}

CardNotInitialized::CardNotInitialized()
    : Exception("Soundscape not initialized")
{}

AutoConfigError::AutoConfigError(const char* msg)
    : Exception("%s", msg)
{}

Timeout::Timeout()
    : Exception("Timeout waiting for firmware reply")
{}

FwError::FwError(std::uint8_t code)
    : Exception("Firmware error %d", code)
{}

UnexpectedEvent::UnexpectedEvent(std::uint8_t code)
    : Exception("Unexpected Event %02x", code)
{}

NewFirmware::NewFirmware()
    : Exception("New SoundScape firmware not supported")
{}

