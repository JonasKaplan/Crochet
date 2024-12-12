#ifndef CROCHET_PARSER_PARSER
#define CROCHET_PARSER_PARSER

#include "../base.h"
#include "nodes.h"

typedef enum {
    PS_OK,
    PS_NO_SUCH_FILE,
    PS_PARSE_ERROR,
} ParserStatus;

ParserStatus parser_parse(NodeSet* set, const char* file);

#endif
