#ifndef CROCHET_CROCHET
#define CROCHET_CROCHET

typedef enum {
    IS_OK,
    IS_ERR,
} InterpreterStatus;

/// @brief Interprets a `.cht` source file
/// @param file The name of the file to interpret
/// @return An `InterpreterStatus` representing weather the interpretation was successful
InterpreterStatus crochet_interpret(const char* file);

#endif
