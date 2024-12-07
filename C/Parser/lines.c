#include "lines.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericStatus lines_line_array_build(LineSequence* sequence, const char* file) {
    FILE* source;
    char current_char;
    Line* line;

    _array_init(sequence, lines, {});
    source = fopen(file, "r");
    if (source == NULL) {
        return GS_NO_SUCH_FILE;
    }
    current_char = '\n';
    do {
        if (current_char == 0xD) {
            continue;
        } else if (current_char == '\n') {
            if (sequence->count == sequence->capacity) {
                _array_extend(sequence, lines, {});
            }
            line = &sequence->lines[sequence->count];
            _array_init(line, chars, lines_line_array_clean(sequence));
            ++sequence->count;
        } else {
            line = &sequence->lines[sequence->count - 1];
            // Check against `capacity - 1` to ensure there's always a null terminator
            if (line->count == line->capacity - 1) {
                _array_extend(line, chars, lines_line_array_clean(sequence));
                memset(&line->chars[line->count], 0, line->capacity - line->count);
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
