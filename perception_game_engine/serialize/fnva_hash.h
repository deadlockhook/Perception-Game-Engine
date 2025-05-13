#pragma once
#include <cstdint>

#define HASHA(str) fnv1a32(str)
#define HASHW(wstr) fnv1a32_wide_full(wstr)

constexpr uint32_t fnv1a32(const char* str, uint32_t hash = 0x811c9dc5) {
    return (str[0] == '\0') ? hash : fnv1a32(str + 1, (hash ^ static_cast<uint8_t>(*str)) * 0x01000193);
}

constexpr uint32_t fnv1a32_wide(const wchar_t* str, uint32_t hash = 0x811c9dc5) {
    return (str[0] == L'\0') ? hash : fnv1a32_wide(str + 1, (hash ^ static_cast<uint8_t>(*str & 0xFF)) * 0x01000193);
}

constexpr uint32_t fnv1a32_wide_full(const wchar_t* str, uint32_t hash = 0x811c9dc5) {
    uint16_t wchar;
    return (str[0] == L'\0') ? hash :
        fnv1a32_wide_full(str + 1,
            (hash ^ (static_cast<uint16_t>(*str) & 0xFF)) * 0x01000193 ^
            (static_cast<uint16_t>(*str) >> 8) & 0xFF);
}
