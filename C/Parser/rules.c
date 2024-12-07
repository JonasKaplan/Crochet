#include "rules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GenericStatus rule_set_find_open(RuleSet* set, u64 id, ActionSequence** out) {
    u8 hashed_id;
    u32 current_table;
    ActionSequence* current_sequence;

    hashed_id = (u8)id;
    current_table = 0;
    while (current_table < set->count) {
        current_sequence = &set->rules[current_table][hashed_id];
        if (current_sequence->actions == NULL) {
            *out = current_sequence;
            return GS_OK;
        }
        ++current_table;
    }
    if (set->count == set->capacity) {
        _array_extend(set, rules, {});
    }
    set->rules[current_table] = calloc(UINT8_MAX, sizeof(*set->rules[current_table]));
    if (set->rules[current_table] == NULL) {
        return GS_OUT_OF_MEMORY;
    }
    ++set->count;
    *out = &set->rules[current_table][hashed_id];
    return GS_OK;
}

GenericStatus rules_rule_set_build(RuleSet* set, const LineSequence* source, u32* last_line_index) {
    u32 line_index, char_index, count;
    u64 constant;
    char buf[MAX_U64_STRING_LENGTH];
    Line* line;
    ActionSequence* sequence;
    GenericStatus status;

    _array_init(set, rules, {});
    set->fallback.actions = NULL;
    line_index = 0;
    while ((line_index < source->count) && (strncmp(source->lines[line_index].chars, "        ", 8) == 0)) {
        line = &source->lines[line_index];
        if (line->count == 0) {
            continue;
        }
        char_index = 8;
        if (line->chars[char_index] == '_') {
            sequence = &set->fallback;
            ++char_index;
            if (!is_whitespace(line->chars[char_index])) {
                rules_rule_set_clean(set);
                return GS_BAD_INPUT;
            }
        } else if (is_digit(line->chars[char_index])) {
            if (sscanf(&line->chars[char_index], "%lu", &constant) != 1) {
                rules_rule_set_clean(set);
                return GS_BAD_INPUT;
            }
            count = 0;
            while (!is_whitespace(line->chars[char_index + count])) {
                ++count;
            }
            memset(buf, 0, MAX_U64_STRING_LENGTH);
            sprintf(buf, "%lu", constant);
            if (strlen(buf) != count) {
                rules_rule_set_clean(set);
                return GS_BAD_INPUT;
            }
            status = rule_set_find_open(set, constant, &sequence);
            sequence->id = constant;
            if (status != GS_OK) {
                rules_rule_set_clean(set);
                return status;
            }
            char_index += count;
        } else {
            rules_rule_set_clean(set);
            return GS_BAD_INPUT;
        }
        while (is_whitespace(line->chars[char_index])) {
            ++char_index;
        }
        if (!((line->chars[char_index] == '-') && (line->chars[char_index + 1] == '>'))) {
            rules_rule_set_clean(set);
            return GS_BAD_INPUT;
        }
        char_index += 3;
        while (is_whitespace(line->chars[char_index])) {
            ++char_index;
        }
        if (line->chars[char_index] == '\0') {
            rules_rule_set_clean(set);
            return GS_BAD_INPUT;
        }
        status = actions_action_sequence_build(sequence, &line->chars[char_index]);
        if (status != GS_OK) {
            rules_rule_set_clean(set);
            return status;
        }
        ++line_index;
    }
    if (set->fallback.actions == NULL) {
        rules_rule_set_clean(set);
        return GS_BAD_INPUT;
    }
    *last_line_index = line_index;
    return GS_OK;
}

void rules_rule_set_clean(RuleSet* set) {
    u32 i;

    while ((set->count--) > 0) {
        for (i = 0; i < UINT8_MAX; ++i) {
            if (set->rules[set->count][i].actions != NULL) {
                actions_action_sequence_clean(&set->rules[set->count][i]);
            }
        }
        free(set->rules[set->count]);
    }
    actions_action_sequence_clean(&set->fallback);
    free(set->rules);
}

const ActionSequence* rules_rule_set_get(const RuleSet* set, u64 id) {
    u8 hashed_id;
    u32 current_table;
    ActionSequence* current_sequence;

    hashed_id = (u8)id; // Yeah. Hashing. For real.
    current_table = 0;
    while (current_table < set->count) {
        current_sequence = &set->rules[current_table][hashed_id];
        if (current_sequence->actions == NULL) {
            return &set->fallback;
        } else if (current_sequence->id == id) {
            return current_sequence;
        }
        ++current_table;
    }
    return &set->fallback;
}
