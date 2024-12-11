#ifndef CROCHET_PARSER_RULE_SET
#define CROCHET_PARSER_RULE_SET

#include "../base.h"
#include "actions.h"
#include "lines.h"

typedef struct {
    u32 count;
    u32 capacity;
    ActionSequence fallback;
    ActionSequence** rules;
} RuleSet;

GenericStatus rules_rule_set_build(RuleSet* set, const LineSequence* source, u32* last_line_index);
void rules_rule_set_clean(RuleSet* set);

const ActionSequence* rules_rule_set_get(const RuleSet* set, u64 id);

#endif
