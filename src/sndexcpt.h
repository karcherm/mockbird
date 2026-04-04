#include <cstdint>
#include <stdexcept>

namespace Soundscape {
class Exception : public std::exception {
public:
    virtual const char* what() const;
protected:
    Exception(const char* format, ...);
private:
    char message_buffer[80];
};

class CardNotPresent : public Exception {
public:
    CardNotPresent(std::uint16_t port);
};

class CardNotInitialized : public Exception {
public:
    CardNotInitialized();
};

class AutoConfigError : public Exception {
public:
    AutoConfigError(const char* message);
};

class FwError : public Exception {
public:
    FwError(std::uint8_t error_code);
};

class UnexpectedEvent : public Exception {
public:
    UnexpectedEvent(std::uint8_t error_code);
};

class Timeout : public Exception {
public:
    Timeout();
};

class NewFirmware : public Exception {
public:
    NewFirmware();
};
}
