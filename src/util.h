#ifndef UTIL_H_INCLUDED_
#define UTIL_H_INCLUDED_
#define ARRAYSIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

const char* skip_spaces(const char* input);
int rescale(int val, int mult, int div);
#endif
