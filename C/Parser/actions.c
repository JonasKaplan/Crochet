#include "actions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GenericStatus action_build(Action* action, u32 start, const char* source) {
    u32 count;
    u64 constant;
    char first_char, operator, buf[MAX_U64_STRING_LENGTH];
    Identifier* identifier;

    count = 0;
    while (!is_whitespace(source[count]) && (source[count] != '\0')) {
        ++count;
    }
    first_char = source[0];
    if (first_char == '!') {
        if (count != 1) {
            fprintf(stderr, "Expected whitespace after `!` near line %u\n", start);
            return GS_BAD_INPUT;
        }
        action->type = AT_SHOW;
        return GS_OK;
    } else if (is_alpha(first_char)) {
        action->type = AT_CALL;
        identifier = &action->identifier;
        identifier->chars = heap_array_M(char, count + 1);
        identifier->count = count;
        memcpy(identifier->chars, source, count);
        return GS_OK;
    }

    switch (first_char) {
        case '+':
            operator = O_ADD;
            source = &source[1];
            --count;
        break;

        case '-':
            operator = O_SUBTRACT;
            source = &source[1];
            --count;
        break;

        case '*':
            operator = O_MULTIPLY;
            source = &source[1];
            --count;
        break;

        case '/':
            operator = O_DIVIDE;
            source = &source[1];
            --count;
        break;

        default:
            operator = O_SET;
        break;
    }

    action->operator = operator;
    if (source[0] == '@') {
        if (count != 1) {
            fprintf(stderr, "Expected whitespace after `@` near line %u\n", start);
            return GS_BAD_INPUT;
        }
        action->type = AT_OPERATE_REF;
    } else if (source[0] == '&') {
        if (count != 1) {
            fprintf(stderr, "Expected whitespace after `&` near line %u\n", start);
            return GS_BAD_INPUT;
        }
        action->type = AT_OPERATE_GET;
    } else if (is_digit(source[0])) {
        if (sscanf(source, "%lu", &constant) != 1) {
            fprintf(stderr, "Bad numeric literal near line %u\n", start);
            return GS_BAD_INPUT;
        }
        memset(buf, 0, MAX_U64_STRING_LENGTH);
        sprintf(buf, "%lu", constant);
        if (strlen(buf) != count) {
            fprintf(stderr, "Bad numeric literal near line %u\n", start);
            return GS_BAD_INPUT;
        }
        action->type = AT_OPERATE_CONST;
        action->constant = constant;
    } else {
        fprintf(stderr, "Unexpected action character near line %u\n", start);
        return GS_BAD_INPUT;
    }
    return GS_OK;
}

GenericStatus actions_action_sequence_build(ActionSequence* sequence, u32 start, const char* source) {
    u32 i;
    GenericStatus status;

    sequence->count = 0;
    sequence->capacity = DEFAULT_ARRAY_CAPACITY;
    sequence->actions = heap_array_M(*sequence->actions, sequence->capacity);
    i = 0;
    while (source[i] != '\0') {
        if (sequence->count == sequence->capacity) {
            resize_array_M(*sequence->actions, sequence->actions, sequence->capacity, 2 * sequence->capacity);
            sequence->capacity *= 2;
        }
        status = action_build(&sequence->actions[sequence->count], start, &source[i]);
        if (status != GS_OK) {
            actions_action_sequence_clean(sequence);
            return status;
        }
        ++sequence->count;
        while ((!is_whitespace(source[i])) && (source[i] != '\0')) {
            ++i;
        }
        while (is_whitespace(source[i]) && (source[i] != '\0')) {
            ++i;
        }
    }
    return GS_OK;
}

void actions_action_sequence_clean(ActionSequence* sequence) {
    while ((sequence->count--) > 0) {
        if (sequence->actions[sequence->count].type == AT_CALL) {
            free(sequence->actions[sequence->count].identifier.chars);
        }
    }
    free(sequence->actions);
    sequence->actions = NULL;
}
