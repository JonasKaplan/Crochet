#include "base.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

bool is_alpha(char c) {
    return (('A' <= c) && (c <= 'Z')) || (('a' <= c) && (c <= 'z'));
}

bool is_digit(char c) {
    return ('0' <= c) && (c <= '9');
}

bool is_alphanum(char c) {
    return is_alpha(c) || is_digit(c) || (c == '_');
}

bool is_whitespace(char c) {
    return (c == 0x9) || (c == 0xA) || (c == 0xD) || (c == 0x20);
}

void* create(u64 block_count, u64 block_size) {
    u8* ptr;

    ptr = calloc(block_count, block_size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* move(void* block, u64 old_block_count, u64 new_block_count, u64 block_size) {
    u8* ptr;

    ptr = realloc(block, new_block_count * block_size);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    if (new_block_count > old_block_count) {
        memset(&ptr[old_block_count * block_size], 0, (new_block_count - old_block_count) * block_size);
    }
    return ptr;
}
