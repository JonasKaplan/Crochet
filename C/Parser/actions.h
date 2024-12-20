#ifndef CROCHET_PARSER_ACTIONS
#define CROCHET_PARSER_ACTIONS

#include "../base.h"

typedef enum {
    AT_SHOW,
    AT_CALL,
    AT_OPERATE_REF,
    AT_OPERATE_GET,
    AT_OPERATE_CONST,
} ActionType;

typedef enum {
    O_SET,
    O_ADD,
    O_SUBTRACT,
    O_MULTIPLY,
    O_DIVIDE,
} Operator;

typedef struct {
    u32 count;
    char* chars;
} Identifier;

typedef struct {
    ActionType type;
    union {
        Identifier identifier;
        struct {
            u64 constant;
            Operator operator;
        };
    };
} Action;

typedef struct {
    u64 id;
    u32 count;
    u32 capacity;
    Action* actions;
} ActionSequence;

GenericStatus actions_action_sequence_build(ActionSequence* sequence, u32 start, const char* source);
void actions_action_sequence_clean(ActionSequence* sequence);

#endif
