#include "C/crochet.h"

int main(void) {
    InterpreterStatus status;
    status = crochet_interpret("Examples/TruthMachine.cht");
    switch (status) {
        case IS_OK:
            return 0;

        case IS_ERR:
            return 1;
    }
}
