#ifndef CROCHET_BASE
#define CROCHET_BASE

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_ARRAY_CAPACITY 16
#define MAX_U64_STRING_LENGTH 21

typedef enum {
    GS_BAD_INPUT,
    GS_NO_SUCH_FILE,
    GS_OK,
    GS_OUT_OF_MEMORY,
} GenericStatus;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

bool is_alpha(char c);
bool is_digit(char c);
bool is_alphanum(char c);
bool is_whitespace(char c);

void* create(u64 block_count, u64 block_size);
void* move(void* block, u64 old_block_count, u64 new_block_count, u64 block_size);

#define heap_M(type)\
    (create(1, sizeof(type)))

#define heap_array_M(type, count)\
    (create((count), sizeof(type)))

#define resize_array_M(type, array, old_count, new_count)\
    ((array) = move((array), (old_count), (new_count), sizeof(type)))

#endif
