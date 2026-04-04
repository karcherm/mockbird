#include "util.h"
#include <cctype>

const char* skip_spaces(const char* input)
{
    while (std::isspace((unsigned char)*input))
        input++;
    return input;
}

int rescale(int val, int mult, int div)
{
    if (val < 0)
        return -(int)(((long)-val * mult + (div >> 1)) / div);
    else
        return (int)(((long)val * mult + (div >> 1)) / div);
}
