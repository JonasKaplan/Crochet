#include "crochet.h"

#include <stdio.h>

int main(int argc, char** argv) {
    InterpreterStatus status;

    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }
    status = crochet_interpret(argv[1]);
    switch (status) {
        case IS_OK:
        return 0;

        case IS_ERR:
        return 1;
    }
}
