#include "actions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GenericStatus action_build(Action* action, const char* source) {
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
            return GS_BAD_INPUT;
        }
        action->type = AT_SHOW;
        return GS_OK;
    } else if (is_alpha(first_char)) {
        action->type = AT_CALL;
        identifier = &action->identifier;
        _array_init_fixed(identifier, chars, count + 1, {});
        memcpy(identifier->chars, source, count);
        --identifier->count;
        return GS_OK;
    }

    if ((first_char == '+') || (first_char == '-') || (first_char == '*') || (first_char == '/')) {
        operator = first_char;
        source = &source[1];
        --count;
    } else {
        operator = '=';
    }
    action->operator = operator;
    if (source[0] == '@') {
        if (count != 1) {
            return GS_BAD_INPUT;
        }
        action->type = AT_OPERATE_REF;
    } else if (is_digit(source[0])) {
        if (sscanf(source, "%lu", &constant) != 1) {
            return GS_BAD_INPUT;
        }
        memset(buf, 0, MAX_U64_STRING_LENGTH);
        sprintf(buf, "%lu", constant);
        if (strlen(buf) != count) {
            return GS_BAD_INPUT;
        }
        action->type = AT_OPERATE_CONST;
        action->constant = constant;
    } else {
        return GS_BAD_INPUT;
    }
    return GS_OK;
}

GenericStatus actions_action_sequence_build(ActionSequence* sequence, const char* source) {
    u32 i;
    GenericStatus status;
    _array_init(sequence, actions, {});
    i = 0;
    while (source[i] != '\0') {
        if (sequence->count == sequence->capacity) {
            _array_extend(sequence, actions, actions_action_sequence_clean(sequence));
        }
        status = action_build(&sequence->actions[sequence->count], &source[i]);
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
}
