#include "nodes.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

static u8 hash(const char* str) {
    // Very very nieve, improve later
    u32 i;
    u8 result;

    i = 0;
    result = 0;
    while (str[i] != '\0') {
        result += str[i];
        ++i;
    }
    return result;
}

static GenericStatus node_set_find_open(NodeSet* set, const char* id, Node** out) {
    u8 hashed_id;
    u32 current_table;
    Node* current_node;

    hashed_id = hash(id);
    current_table = 0;
    while (current_table < set->count) {
        current_node = &set->nodes[current_table][hashed_id];
        if (current_node->spawn_rules.rules == NULL) {
            *out = current_node;
            return GS_OK;
        }
        ++current_table;
    }
    if (set->count == set->capacity) {
        _array_extend(set, nodes, {});
    }
    set->nodes[current_table] = calloc(UINT8_MAX, sizeof(*set->nodes[current_table]));
    if (set->nodes[current_table] == NULL) {
        return GS_OUT_OF_MEMORY;
    }
    ++set->count;
    *out = &set->nodes[current_table][hashed_id];
    return GS_OK;
}

GenericStatus nodes_node_set_build(NodeSet* set, const LineSequence* sequence) {
    Line* line;
    char* identifier;
    Node* node;
    LineSequence sub_sequence;
    u32 i, j, k, l, m, line_index, last_line_index, count;
    GenericStatus status;

    _array_init(set, nodes, {});
    line_index = 0;
    while (line_index < sequence->count) {
        line = &sequence->lines[line_index];
        if ((line->chars[0] == '#') || (line->count == 0)) {
            ++line_index;
            continue;
        }
        if (!is_alpha(line->chars[0])) {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        count = 0;
        while ((!is_whitespace(line->chars[count])) && (line->chars[count] != '\0')) {
            ++count;
        }
        if (line->chars[count] != '\0') {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        identifier = line->chars;
        node = nodes_node_set_get(set, identifier);
        if (node != NULL) {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        status = node_set_find_open(set, identifier, &node);
        if (status != GS_OK) {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        node->name = calloc(line->count + 1, sizeof(*node->name));
        if (node->name == NULL) {
            nodes_node_set_clean(set);
            return GS_OUT_OF_MEMORY;
        }
        strcpy(node->name, line->chars);

        ++line_index;
        while ((line_index != sequence->count) && (sequence->lines[line_index].count == 0)) {
            ++line_index;
        }
        if ((line_index == sequence->count) || (strcmp(sequence->lines[line_index].chars, "    spawn") != 0)) {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        ++line_index;
        while ((line_index != sequence->count) && (sequence->lines[line_index].count == 0)) {
            ++line_index;
        }
        sub_sequence = (LineSequence){.lines = &sequence->lines[line_index], .count = sequence->count - line_index};
        status = rules_rule_set_build(&node->spawn_rules, &sub_sequence, &last_line_index);
        if ((status != GS_OK) || (last_line_index == sequence->count)) {
            nodes_node_set_clean(set);
            return status;
        }
        line_index += last_line_index;

        while ((line_index != sequence->count) && (sequence->lines[line_index].count == 0)) {
            ++line_index;
        }
        if ((line_index == sequence->count) || (strcmp(sequence->lines[line_index].chars, "    pop") != 0)) {
            nodes_node_set_clean(set);
            return GS_BAD_INPUT;
        }
        ++line_index;
        while ((line_index != sequence->count) && (sequence->lines[line_index].count == 0)) {
            ++line_index;
        }
        sub_sequence = (LineSequence){.lines = &sequence->lines[line_index], .count = sequence->count - line_index};
        status = rules_rule_set_build(&node->pop_rules, &sub_sequence, &last_line_index);
        if ((status != GS_OK) || (last_line_index == sequence->count)) {
            nodes_node_set_clean(set);
            return status;
        }
        line_index += last_line_index;
    }
    for (i = 0; i < set->count; ++i) {
        for (j = 0; j < UINT8_MAX; ++j) {
            if (set->nodes[i][j].name != NULL) {
                node = &set->nodes[i][j];
                for (k = 0; k < node->spawn_rules.count; ++k) {
                    for (l = 0; l < UINT8_MAX; ++l) {
                        if (node->spawn_rules.rules[k][l].actions != NULL) {
                            for (m = 0; m < node->spawn_rules.rules[k][l].count; ++m) {
                                if ((node->spawn_rules.rules[k][l].actions[m].type == AT_CALL) && (nodes_node_set_get(set, node->spawn_rules.rules[k][l].actions[m].identifier.chars) == NULL)) {
                                    nodes_node_set_clean(set);
                                    return GS_BAD_INPUT;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (nodes_node_set_get(set, "origin") == NULL) {
        nodes_node_set_clean(set);
        return GS_BAD_INPUT;
    }
    free(set->nodes[set->count]);
    return GS_OK;
}

void nodes_node_set_clean(NodeSet* set) {
    u32 i;

    while ((set->count--) > 0) {
        for (i = 0; i < UINT8_MAX; ++i) {
            if (set->nodes[set->count][i].name != NULL) {
                free(set->nodes[set->count][i].name);
                rules_rule_set_clean(&set->nodes[set->count][i].spawn_rules);
                rules_rule_set_clean(&set->nodes[set->count][i].pop_rules);
            }
        }
        free(set->nodes[set->count]);
    }
    free(set->nodes);
}

Node* nodes_node_set_get(const NodeSet* set, const char* id) {
    u8 hashed_id;
    u32 current_table;
    Node* current_node;

    hashed_id = hash(id);
    current_table = 0;
    while (current_table < set->count) {
        current_node = &set->nodes[current_table][hashed_id];
        if (current_node->name == NULL) {
            return NULL;
        } else if (strcmp(current_node->name, id) == 0) {
            return current_node;
        }
        ++current_table;
    }
    return NULL;
}
