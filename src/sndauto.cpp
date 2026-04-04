// Initialization from SNDSCAPE.INI
// this incurs big overhead, because it pulls the ini parser that
// furthermore pulls the C++ string library and C++ I/O stream library
// as well as generating some std::map instances.
#include "sndscape.h"
#include "sndexcpt.h"
#include "sndpriv.h"
#include "ini.h"
#include <cstdlib>
#include <string>

using namespace Soundscape;

static std::uint16_t get_ini_port()
{
    const char* sndscape = std::getenv("SNDSCAPE");
    if (!sndscape)
        throw AutoConfigError("SNDSCAPE variable not set");
    std::string path(sndscape);
    if (path[path.size()-1] != '\\')
        path.push_back('\\');
    path.append("SNDSCAPE.INI");
    IniData ini = ReadIniFile(path.c_str());
    IniData::iterator sndscape_drv = ini.find("sndscape.drv");
    if (sndscape_drv == ini.end())
        throw AutoConfigError("Section sndscape.drv missing in SNDSCAPE.INI");
    IniSection::iterator port = sndscape_drv->second.find("port");
    if (port == sndscape_drv->second.end())
        throw AutoConfigError("Port entry missing in SNDSCAPE.INI");
    unsigned portval;
    char tmp;
    if (std::sscanf(port->second.c_str(), "%x%c", &portval, &tmp) != 1)
        throw AutoConfigError("Port entry malformed in SNDSCAPE.INI");
    return portval;
}

Card::Card()
    : base_port(get_ini_port()),
      midi_port(base_port + PRIM_CHANNEL),
      host_port(_ensure_booted(base_port) + SEC_CHANNEL, true)
{
    _ctor_common();
}

