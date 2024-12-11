#include "crochet.h"

#include <stdio.h>
#include <stdlib.h>
#include "Parser/parser.h"

#define STACK_SIZE 8192

typedef struct {
    const Node* node;
    u64 value;
    u32 active_children;
    u32 capacity;
    u64* results;
} RuntimeNode;

static GenericStatus node_spawn(const Node* node, const NodeSet* set, u64 value, u16 depth, u64* result);

static GenericStatus node_process_action(RuntimeNode* node, Action* action, u64 trigger, u16 depth, const NodeSet* set) {
    u64 operand;

    switch (action->type) {
        case AT_SHOW:
            printf("%lu\n", node->value);
        return GS_OK;

        case AT_CALL:
            if (node->active_children == node->capacity) {
                resize_array_M(*node->results, node->results, node->capacity, 2 * node->capacity);
                node->capacity *= 2;
            }
            node_spawn(nodes_node_set_get(set, action->identifier.chars), set, node->value, depth + 1, &node->results[node->active_children]);
            ++node->active_children;
        return GS_OK;

        case AT_OPERATE_REF:
            operand = trigger;
        break;

        case AT_OPERATE_GET:
            operand = getchar();
        break;

        case AT_OPERATE_CONST:
            operand = action->constant;
        break;
    }

    switch (action->operator) {
        case '=':
            node->value = operand;
        return GS_OK;

        case '+':
            node->value += operand;
        return GS_OK;

        case '-':
            node->value -= operand;
        return GS_OK;

        case '*':
            node->value *= operand;
        return GS_OK;

        case '/':
            if (operand == 0) {
                return GS_BAD_INPUT;
            }
            node->value /= operand;
        return GS_OK;
    }

    return GS_BAD_INPUT;
}

static GenericStatus node_spawn(const Node* node, const NodeSet* set, u64 value, u16 depth, u64* result) {
    if (depth == STACK_SIZE) {
        fprintf(stderr, "Stack overflow\n");
        // Very temporary, fix later
        exit(EXIT_FAILURE);
    }
    RuntimeNode runtime_node;
    const ActionSequence* sequence;
    GenericStatus status;
    u32 i;

    runtime_node.node = node;
    runtime_node.value = value;
    runtime_node.active_children = 0;
    runtime_node.capacity = DEFAULT_ARRAY_CAPACITY;
    runtime_node.results = heap_array_M(*runtime_node.results, runtime_node.capacity);

    sequence = rules_rule_set_get(&node->spawn_rules, runtime_node.value);
    for (i = 0; i < sequence->count; ++i) {
        status = node_process_action(&runtime_node, &sequence->actions[i], value, depth, set);
        if (status != GS_OK) {
            return status;
        }
    }

    while (runtime_node.active_children != 0) {
        --runtime_node.active_children;
        sequence = rules_rule_set_get(&node->pop_rules, runtime_node.results[runtime_node.active_children]);
        for (i = 0; i < sequence->count; ++i) {
            status = node_process_action(&runtime_node, &sequence->actions[i], runtime_node.results[runtime_node.active_children], depth, set);
            if (status != GS_OK) {
                return status;
            }
        }
    }
    *result = runtime_node.value;
    free(runtime_node.results);
    return GS_OK;
}

InterpreterStatus crochet_interpret(const char* file) {
    NodeSet set;
    ParserStatus parser_status;
    GenericStatus status;
    u64 result;

    parser_status = parser_parse(&set, file);
    if (parser_status == PS_OUT_OF_MEMORY) {
        fprintf(stderr, "Out of memory\n");
    } else if (parser_status == PS_NO_SUCH_FILE) {
        fprintf(stderr, "No such file \"%s\"\n", file);
    } else if (parser_status == PS_PARSE_ERROR) {
        fprintf(stderr, "Parsing error\n");
    }
    if (parser_status != PS_OK) {
        return IS_ERR;
    }

    status = node_spawn(nodes_node_set_get(&set, "origin"), &set, 0, 0, &result);
    if (status != GS_OK) {
        fprintf(stderr, "Runtime error\n");
        return IS_ERR;
    }
    nodes_node_set_clean(&set);
    return (result == 0) ? IS_OK : IS_ERR;
}
