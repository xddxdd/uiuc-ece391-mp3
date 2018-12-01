#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    if(-1 == ece391_reboot()) {
        ece391_fdputs (1, (uint8_t*) "reboot not supported by hardware\n");
    }

    return 0;
}
