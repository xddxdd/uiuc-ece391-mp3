#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    if(-1 == ece391_shutdown()) {
        ece391_fdputs (1, (uint8_t*) "shutdown not supported by hardware\n");
    }

    return 0;
}
