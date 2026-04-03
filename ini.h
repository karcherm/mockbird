#include <map>
#include <string>
#include <stdexcept>

typedef std::map<std::string, std::string> IniSection;
typedef std::map<std::string, IniSection > IniData;

IniData ReadIniFile(const char* path);
