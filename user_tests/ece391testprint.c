#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{

    ece391_fdputs (1, (uint8_t*)"Hello, if this ran, then Max is Superb! Yay!\n");

    return 0;
}

