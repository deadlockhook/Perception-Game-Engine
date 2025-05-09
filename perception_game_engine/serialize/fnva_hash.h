#pragma once
#include <cstdint>

#define HASHA(str) fnv1a32(str)
#define HASHW(wstr) fnv1a32_wide_full(wstr)

__forceinline uint32_t fnv1a32(const char* str) {
	const uint32_t fnv_prime = 0x01000193;
	uint32_t hash = 0x811c9dc5;

	while (*str) {
		hash ^= static_cast<uint8_t>(*str++);
		hash *= fnv_prime;
	}
	return hash;
}

__forceinline uint32_t fnv1a32_wide(const wchar_t* str) {
	const uint32_t fnv_prime = 0x01000193;
	uint32_t hash = 0x811c9dc5;

	while (*str) {
		hash ^= static_cast<uint8_t>(*str++ & 0xFF);
		hash *= fnv_prime;
	}
	return hash;
}

__forceinline uint32_t fnv1a32_wide_full(const wchar_t* str) {
	const uint32_t fnv_prime = 0x01000193;
	uint32_t hash = 0x811c9dc5;

	while (*str) {
		uint16_t wchar = static_cast<uint16_t>(*str++);
		hash ^= (wchar & 0xFF);         
		hash *= fnv_prime;
		hash ^= ((wchar >> 8) & 0xFF);   
		hash *= fnv_prime;
	}
	return hash;
}
