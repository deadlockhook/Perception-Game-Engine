#pragma once
#include <cstdint>

typedef union _virtual_address_t
{
    uint64_t value;
    struct
    {
        uint64_t offset : 12;
        uint64_t pt_index : 9;
        uint64_t pd_index : 9;
        uint64_t pdpt_index : 9;
        uint64_t pml4_index : 9;
        uint64_t reserved : 16;
    };
} virtual_address_t;

