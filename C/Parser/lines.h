#ifndef CROCHET_PARSER_LINES
#define CROCHET_PARSER_LINES

#include "../base.h"

typedef struct {
    u32 count;
    u32 capacity;
    char* chars;
} Line;

typedef struct {
    u32 count;
    u32 capacity;
    Line* lines;
} LineSequence;

GenericStatus lines_line_array_build(LineSequence* sequence, const char* file);
void lines_line_array_clean(LineSequence* sequence);

#endif
