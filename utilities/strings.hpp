#ifndef VRITA_STRINGS_INCLUDES
#define VRITA_STRINGS_INCLUDES

#include <iostream>
#include <cstdio>

inline std::string concatOpcodeStrings(const char* str1, const char* str2) { return std::string(str1) + std::string(str2); }
inline const char* concatOpcodeStringsC(const char* str1, const char* str2) { thread_local char buf[512]; snprintf(buf, sizeof(buf), "%s%s", str1, str2); return buf; }

#endif