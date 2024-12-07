#ifndef CROCHET_CROCHET
#define CROCHET_CROCHET

typedef enum {
    IS_OK,
    IS_ERR,
} InterpreterStatus;

InterpreterStatus crochet_interpret(const char* file);

#endif
