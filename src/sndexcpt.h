#include <cstdint>
#include <stdexcept>

namespace Soundscape {
/**
 * (internal) base exception class for Soundscape exceptions.
 *
 * Do not catch \c Exception. Catch `std::exception` if you want to catch all, or
 * specific derived exceptions if you want to catch specific errors. Soundscape methods
 * may throw other exceptions derived from `std::exception` as well.
 */
class Exception : public std::exception {
public:
    /**
     * Returns a user-readable message describing the exceptional condition
     *
     * @return error message
     */
    virtual const char* what() const;
protected:
    /** 
     * printf-style exception constructor.
     *
     * Constructs an exception with a fixed 80-character error message buffer
     * using \c sprintf.
     *
     * \param format  printf-style format string
     * \param ...     items for the format string
     */
    Exception(const char* format, ...);
private:
    char message_buffer[80];
};

/// The Soundscape hardware is not present in the system
class CardNotPresent : public Exception {
public:
    /**
     * Creates the exception given the base I/O port.
     *
     * @param port  The base I/O port (0x320 to 0x350)
     */
    CardNotPresent(std::uint16_t port);
};

/// The Soundscape hardware is not initialized (using SSINIT)
class CardNotInitialized : public Exception {
public:
    /// Creates the exception.
    CardNotInitialized();
};

/// Loading configuration from SNDSCAPE.INI failed
class AutoConfigError : public Exception {
public:
    /**
     * Creates the exception, containing an error from the load attempt
     *
     * @param message  The lower level error message (max 80 characters)
     */
    AutoConfigError(const char* message);
};

/// Soundscape firmware responded with an error code
class FwError : public Exception {
public:
    /**
     * Creates the exception, containing a firmware-reported error code
     *
     * @param error_code  The error code as reported by the firmware
     */
    FwError(std::uint8_t error_code);
};

/// Soundscape firmware reported an unexpected event
class UnexpectedEvent : public Exception {
public:
    /**
     * Creates the exception, containing a firmware-reported event code
     *
     * @param event  The event code
     */
    UnexpectedEvent(std::uint8_t event);
};

/// Timeout waiting for readiness or response
class Timeout : public Exception {
public:
    /// Creates the exception
    Timeout();
};

/**
 * The Soundscape card is initialized by the "new" driver pack.
 *
 * The "old" driverpack contains the Mockingbird-OTTO firmware. This is likely the only
 * driver pack to exist for S-1000-based cards. For S-2000-based cards, only driver pack
 * 1.0 (containing `SNDSCAPE.COD`) contains the synthesizer that supports custom patches,
 * while the driver packs starting at 1.2 do contain firmwares with fixed sample mappings
 * named `SNDSCAPE.CO0` and alike. Note that the firmware included in the S-2000 driver
 * pack 1.0 reports an internal version number of 1.21. This firmware is OK.
 */
class NewFirmware : public Exception {
public:
    /// Creates the excpetion
    NewFirmware();
};
}
