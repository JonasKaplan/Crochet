#ifndef CROCHET_PARSER_NODES
#define CROCHET_PARSER_NODES

#include "../base.h"
#include "rules.h"
#include "lines.h"

typedef struct {
    char* name;
    RuleSet spawn_rules;
    RuleSet pop_rules;
} Node;

typedef struct {
    u32 count;
    u32 capacity;
    Node** nodes;
} NodeSet;

GenericStatus nodes_node_set_build(NodeSet* set, const LineSequence* sequence);
void nodes_node_set_clean(NodeSet* set);

Node* nodes_node_set_get(const NodeSet* set, const char* id);

#endif
