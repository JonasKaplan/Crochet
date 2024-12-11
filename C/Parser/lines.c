#include "lines.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericStatus lines_line_array_build(LineSequence* sequence, const char* file) {
    FILE* source;
    char current_char;
    Line* line;

    sequence->count = 0;
    sequence->capacity = DEFAULT_ARRAY_CAPACITY;
    sequence->lines = heap_array_M(Line, sequence->capacity);
    source = fopen(file, "r");
    if (source == NULL) {
        free(sequence->lines);
        return GS_NO_SUCH_FILE;
    }
    current_char = '\n';
    do {
        if (current_char == 0xD) {
            continue;
        } else if (current_char == '\n') {
            if (sequence->count == sequence->capacity) {
                resize_array_M(Line, sequence->lines, sequence->capacity, 2 * sequence->capacity);
                sequence->capacity *= 2;
            }
            line = &sequence->lines[sequence->count];
            line->count = 0;
            line->capacity = DEFAULT_ARRAY_CAPACITY;
            line->chars = heap_array_M(char, line->capacity);
            ++sequence->count;
        } else {
            // Check against `capacity - 1` to ensure there's always a null terminator
            if (line->count == line->capacity - 1) {
                resize_array_M(char, line->chars, line->capacity, line->capacity * 2);
                line->capacity *= 2;
            }
            line->chars[line->count++] = current_char;
        }
    } while ((current_char = fgetc(source)) != EOF);
    fclose(source);
    return GS_OK;
}

void lines_line_array_clean(LineSequence* sequence) {
    while ((sequence->count--) > 0) {
        free(sequence->lines[sequence->count].chars);
    }
    free(sequence->lines);
}
