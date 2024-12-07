#include "base-defs.h"

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
