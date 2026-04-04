#include "ini.h"
#include <istream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <stdexcept>

#ifdef __WATCOMC__
std::istream& mygetline(std::istream& stream, std::string& target)
{
    target.clear();
    while (true)
    {
        // OpenWatcom 1.9 does not have std::istream::traits_type
        typedef std::char_traits<char> traits_t;
        size_t read_start = target.size();
        target.resize(read_start + 80);
        stream.get((char*)target.data() + read_start, 80);
        target.resize(target.find('\0'));
        traits_t::int_type maybe_delim = stream.get();
        if (maybe_delim == traits_t::eof())  // EOF
            break;
        target.push_back(traits_t::to_char_type(maybe_delim));
        if (maybe_delim == '\n')
            break;
    }
    return stream;
}
#else
#define mygetline std::getline
#endif

inline char mytolower(char c)
{
    return (char)std::tolower((unsigned char)c);
}

IniData ReadIniFile(const char* path)
{
    try {
        std::fstream inifile(path, std::ios::in);
        inifile.exceptions(std::ios::badbit);
        IniData data;
        IniSection *current_section = NULL;
        std::string line;
        while (mygetline(inifile, line))
        {
            while (!line.empty() &&
                   std::isspace((unsigned char)line[line.size() - 1]))
                line.resize(line.size() - 1); // OpenWatcom 1.9 misses pop_back
            if (line.empty())
                continue;
            if (line[0] == ';')
                continue;
            if (line[0] == '[')
            {
                if (line[line.size()-1] != ']')
                    continue;
                std::transform(line.begin(), line.end(), line.begin(), mytolower);
                current_section = &data[line.substr(1, line.size()-2)];
            }
            if (current_section == NULL)
                continue;
            size_t equalpos = line.find('=');
            if (equalpos == std::string::npos)
                continue;
            line[equalpos] = '\0';
            std::transform(line.begin(), line.begin() + equalpos, line.begin(), mytolower);
            (*current_section)[line.substr(0, equalpos)] = line.substr(equalpos+1);
        }
        return data;
    } catch(std::ios::failure& f) {
        throw std::runtime_error("IO error reading INI file");
    }
}

