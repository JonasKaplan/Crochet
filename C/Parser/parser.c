#include "parser.h"

#include <stdlib.h>

ParserStatus parser_parse(NodeSet* set, const char* file) {
    LineSequence sequence;
    GenericStatus status;

    status = lines_line_array_build(&sequence, file);
    if (status == GS_NO_SUCH_FILE) {
        return PS_NO_SUCH_FILE;
    }
    status = nodes_node_set_build(set, &sequence);
    if (status == GS_BAD_INPUT) {
        lines_line_array_clean(&sequence);
        return PS_PARSE_ERROR;
    } else {
        lines_line_array_clean(&sequence);
        return PS_OK;
    }
}
