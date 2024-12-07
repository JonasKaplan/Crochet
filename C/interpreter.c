#include "crochet.h"

#include <stdio.h>
#include <stdlib.h>
#include "Parser/parser.h"

typedef struct _QueueNode {
    u64 value;
    struct _QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* head;
    QueueNode* tail;
} Queue;

typedef enum {
    QS_OK,
    QS_OUT_OF_MEMORY,
    QS_EMPTY,
} QueueStatus;

static QueueStatus queue_push(Queue* queue, u64 value) {
    QueueNode *new_node, *other;

    new_node = calloc(1, sizeof(*new_node));
    if (new_node == NULL) {
        new_node = queue->head;
        while (new_node != NULL) {
            other = new_node->next;
            free(new_node);
            new_node = other;
        }
        return QS_OUT_OF_MEMORY;
    }
    new_node->value = value;
    new_node->next = NULL;
    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
        return QS_OK;
    }
    queue->tail->next = new_node;
    queue->tail = new_node;
    return QS_OK;
}

static QueueStatus queue_pop(Queue* queue, u64* value) {
    if (queue->head == NULL) {
        return QS_EMPTY;
    }
    QueueNode* popped;

    popped = queue->head;
    if (queue->head == queue->tail) {
        queue->tail = NULL;
    }
    queue->head = popped->next;
    *value = popped->value;
    free(popped);
    return QS_OK;
}


static GenericStatus node_spawn(const Node* node, const NodeSet* set, u64 value, u64* result) {
    u32 i;
    u64 current_result, current_value;
    const ActionSequence *spawn_rule, *pop_rule;
    Action* action;
    GenericStatus status;
    Queue returned;

    current_value = value;
    returned.head = NULL;
    returned.tail = NULL;
    spawn_rule = rules_rule_set_get(&node->spawn_rules, current_value);
    for (i = 0; i < spawn_rule->count; ++i) {
        action = &spawn_rule->actions[i];
        switch (action->type) {
            case AT_SHOW:
                printf("%lu\n", current_value);
            break;

            case AT_CALL:
                status = node_spawn(nodes_node_set_get(set, action->identifier.chars), set, current_value, &current_result);
                if (status != GS_OK) {
                    return status;
                }
                queue_push(&returned, current_result);
            break;

            case AT_OPERATE_REF:
                switch (action->operator) {
                    case '=':
                        current_value = value;
                    break;

                    case '+':
                        current_value += value;
                    break;

                    case '-':
                        current_value -= value;
                    break;

                    case '*':
                        current_value *= value;
                    break;

                    case '/':
                        current_value /= value;
                    break;
                }
            break;

            case AT_OPERATE_CONST:
                switch (action->operator) {
                    case '=':
                        current_value = action->constant;
                    break;

                    case '+':
                        current_value += action->constant;
                    break;

                    case '-':
                        current_value -= action->constant;
                    break;

                    case '*':
                        current_value *= action->constant;
                    break;

                    case '/':
                        current_value /= action->constant;
                    break;
                }
            break;
        }
    }
    while (queue_pop(&returned, &current_result) == QS_OK) {
        pop_rule = rules_rule_set_get(&node->pop_rules, current_result);
        for (i = 0; i < pop_rule->count; ++i) {
            action = &pop_rule->actions[i];
            switch (action->type) {
                case AT_SHOW:
                    printf("%lu\n", current_value);
                break;

                case AT_CALL:
                    status = node_spawn(nodes_node_set_get(set, action->identifier.chars), set, current_value, &current_result);
                    if (status != GS_OK) {
                        return status;
                    }
                    queue_push(&returned, current_result);
                break;

                case AT_OPERATE_REF:
                    switch (action->operator) {
                        case '=':
                            current_value = current_result;
                        break;

                        case '+':
                            current_value += current_result;
                        break;

                        case '-':
                            current_value -= current_result;
                        break;

                        case '*':
                            current_value *= current_result;
                        break;

                        case '/':
                            current_value /= current_result;
                        break;
                    }
                break;

                case AT_OPERATE_CONST:
                    switch (action->operator) {
                        case '=':
                            current_value = action->constant;
                        break;

                        case '+':
                            current_value += action->constant;
                        break;

                        case '-':
                            current_value -= action->constant;
                        break;

                        case '*':
                            current_value *= action->constant;
                        break;

                        case '/':
                            current_value /= action->constant;
                        break;
                    }
                break;
            }
        }
    }
    *result = current_value;
    return GS_OK;
}

InterpreterStatus crochet_interpret(const char* file) {
    NodeSet set;
    ParserStatus parser_status;
    GenericStatus status;
    u64 result;

    parser_status = parser_parse(&set, file);
    if (parser_status == PS_OUT_OF_MEMORY) {
        printf("Out of memory\n");
    } else if (parser_status == PS_NO_SUCH_FILE) {
        printf("No such file \"%s\"\n", file);
    } else if (parser_status == PS_PARSE_ERROR) {
        printf("Parsing error\n");
    }
    if (parser_status != PS_OK) {
        return IS_ERR;
    }

    status = node_spawn(nodes_node_set_get(&set, "origin"), &set, 0, &result);
    if (status != GS_OK) {
        printf("Runtime error\n");
        return IS_ERR;
    }
    nodes_node_set_clean(&set);
    return (result == 0) ? IS_OK : IS_ERR;
}
