#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    if(-1 == ece391_ps()) {
        ece391_fdputs (1, (uint8_t*) "ps not supported by system\n");
    }

    return 0;
}
