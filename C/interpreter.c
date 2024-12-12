#include "crochet.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "Parser/parser.h"

#define STACK_SIZE 256

typedef struct _RuntimeNode RuntimeNode;
typedef struct _ThreadArgs ThreadArgs;

struct _RuntimeNode {
    const Node* node;
    u64 value;
    u32 inactive_children;
    u32 active_children;
    u32 child_capacity;
    pthread_t* children;
    ThreadArgs* child_args;
    bool error;
    sem_t mutex;
};

struct _ThreadArgs {
    const NodeSet* set;
    const Node* node;
    RuntimeNode* parent;
    u64 spawn_value;
    u32 depth;
};

void* node_thread(void* args);

void node_apply_sequence(RuntimeNode* runtime_node, const ActionSequence* sequence, u64 initial_value, u32 depth, const NodeSet* set) {
    Action* action;
    u32 i;
    u64 operand;
    ThreadArgs* args;
    bool operate;
    for (i = 0; i < sequence->count; ++i) {
        action = &sequence->actions[i];
        operate = false;
        switch (action->type) {
            case AT_SHOW:
                printf("%lu\n", runtime_node->value);
            break;

            case AT_CALL:
                if ((runtime_node->active_children + runtime_node->inactive_children) == runtime_node->child_capacity) {
                    resize_array_M(*runtime_node->children, runtime_node->children, runtime_node->child_capacity, 2 * runtime_node->child_capacity);
                    resize_array_M(*runtime_node->child_args, runtime_node->child_args, runtime_node->child_capacity, 2 * runtime_node->child_capacity);
                    runtime_node->child_capacity *= 2;
                }
                args = &runtime_node->child_args[runtime_node->active_children + runtime_node->inactive_children];
                args->set = set;
                args->node = nodes_node_set_get(set, action->identifier.chars);
                args->parent = runtime_node;
                args->spawn_value = runtime_node->value;
                args->depth = depth + 1;
                pthread_create(&runtime_node->children[runtime_node->active_children + runtime_node->inactive_children], NULL, node_thread, args);
                ++runtime_node->active_children;
            break;

            case AT_OPERATE_REF:
                operate = true;
                operand = initial_value;
            break;

            case AT_OPERATE_GET:
                operate = true;
                operand = getchar();
            break;

            case AT_OPERATE_CONST:
                operate = true;
                operand = action->constant;
            break;
        }
        if (!operate) {
            continue;
        }
        switch (action->operator) {
            case O_SET:
                runtime_node->value = operand;
            break;

            case O_ADD:
                runtime_node->value += operand;
            break;

            case O_SUBTRACT:
                runtime_node->value -= operand;
            break;

            case O_MULTIPLY:
                runtime_node->value *= operand;
            break;

            case O_DIVIDE:
                runtime_node->value /= operand;
            break;
        }
    }
}

void* node_thread(void* args) {
    ThreadArgs* thread_args;
    const ActionSequence* sequence;
    RuntimeNode runtime_node;
    u32 i;

    thread_args = (ThreadArgs*)args;

    if (thread_args->depth == STACK_SIZE) {
        fprintf(stderr, "Stack overflow\n");
        sem_wait(&thread_args->parent->mutex);
        --thread_args->parent->active_children;
        ++thread_args->parent->inactive_children;
        thread_args->parent->error = true;
        sem_post(&thread_args->parent->mutex);
        pthread_exit(NULL);
    }

    runtime_node.node = thread_args->node;
    runtime_node.value = 0;
    runtime_node.inactive_children = 0;
    runtime_node.active_children = 0;
    runtime_node.child_capacity = DEFAULT_ARRAY_CAPACITY;
    runtime_node.children = heap_array_M(*runtime_node.children, runtime_node.child_capacity);
    runtime_node.child_args = heap_array_M(*runtime_node.child_args, runtime_node.child_capacity);
    runtime_node.error = false;
    sem_init(&runtime_node.mutex, 0, 0);

    sequence = rules_rule_set_get(&runtime_node.node->spawn_rules, thread_args->spawn_value);
    node_apply_sequence(&runtime_node, sequence, thread_args->spawn_value, thread_args->depth, thread_args->set);

    sem_post(&runtime_node.mutex);
    while (true) {
        sem_wait(&runtime_node.mutex);
        if (runtime_node.active_children == 0) {
            break;
        }
        sem_post(&runtime_node.mutex);
    }

    sem_wait(&thread_args->parent->mutex);
    if (runtime_node.error) {
        thread_args->parent->error = true;
    } else {
        sequence = rules_rule_set_get(&thread_args->parent->node->pop_rules, runtime_node.value);
        node_apply_sequence(thread_args->parent, sequence, runtime_node.value, thread_args->depth - 1, thread_args->set);
    }
    --thread_args->parent->active_children;
    ++thread_args->parent->inactive_children;
    sem_post(&thread_args->parent->mutex);

    for (i = 0; i < runtime_node.inactive_children; ++i) {
        pthread_join(runtime_node.children[i], NULL);
    }

    free(runtime_node.children);
    free(runtime_node.child_args);
    sem_destroy(&runtime_node.mutex);
    pthread_exit(NULL);
}

InterpreterStatus crochet_interpret(const char* file) {
    NodeSet set;
    ParserStatus status;
    RuntimeNode origin;
    const ActionSequence* sequence;
    u32 i;

    status = parser_parse(&set, file);
    if (status == PS_NO_SUCH_FILE) {
        fprintf(stderr, "No such file \"%s\"\n", file);
    } else if (status == PS_PARSE_ERROR) {
        fprintf(stderr, "Failed to parse source file \"%s\"\n", file);
    }
    if (status != PS_OK) {
        return IS_ERR;
    }

    origin.node = nodes_node_set_get(&set, "origin");
    origin.value = 0;
    origin.inactive_children = 0;
    origin.active_children = 0;
    origin.child_capacity = DEFAULT_ARRAY_CAPACITY;
    origin.children = heap_array_M(*origin.children, origin.child_capacity);
    origin.child_args = heap_array_M(*origin.child_args, origin.child_capacity);
    origin.error = false;
    sem_init(&origin.mutex, 0, 0);

    sequence = rules_rule_set_get(&origin.node->spawn_rules, 0);
    node_apply_sequence(&origin, sequence, 0, 0, &set);

    sem_post(&origin.mutex);
    while (true) {
        sem_wait(&origin.mutex);
        if (origin.active_children == 0) {
            break;
        }
        sem_post(&origin.mutex);
    }

    for (i = 0; i < origin.inactive_children; ++i) {
        pthread_join(origin.children[i], NULL);
    }

    free(origin.children);
    free(origin.child_args);
    sem_destroy(&origin.mutex);
    nodes_node_set_clean(&set);
    return (origin.error || (origin.value != 0)) ? IS_ERR : IS_OK;
}
