#ifndef CROCHET_BASE_DEFS
#define CROCHET_BASE_DEFS

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

#define _array_init(container, array, cleanup)\
container->count = 0;\
container->capacity = DEFAULT_ARRAY_CAPACITY;\
container->array = calloc(container->capacity, sizeof(*container->array));\
if (container->array == NULL) { cleanup; return GS_OUT_OF_MEMORY; }

#define _array_init_fixed(container, array, _count, cleanup)\
container->count = _count;\
container->capacity = _count;\
container->array = calloc(_count, sizeof(*container->array));\
if (container->array == NULL) { cleanup; return GS_OUT_OF_MEMORY; }

#define _array_extend(container, array, cleanup)\
container->capacity *= 2;\
container->array = realloc(container->array, container->capacity * sizeof(*container->array));\
if (container->array == NULL) { cleanup; return GS_OUT_OF_MEMORY; }

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

#endif
