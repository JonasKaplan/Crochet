#include "C/crochet.h"

int main(int argc, char** argv) {
    InterpreterStatus status;
    status = crochet_interpret(argv[1]);
    switch (status) {
        case IS_OK:
            return 0;

        case IS_ERR:
            return 1;
    }
}
