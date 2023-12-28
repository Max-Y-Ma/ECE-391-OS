
#include "lib391/ece391syscall.h"
#include "lib391/ece391support.h"
#include "lib391/ece391sysnum.h"


/* I'm gonna try hard-coding this for now. */


int main() {

    int32_t fd, cnt;
    uint8_t name [1024];
    uint8_t buf[1024];
    ece391_strcpy(buf, "I want to write a string here \n ");
    /* So far, it just takes a file name, but no string. Hence, the format should be "write_test [filename], and it hard-writes ths string." */
    int res;
    if (0 != (res = ece391_getargs (name, 1024) ) ) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }
   

    if (-1 == (fd = ece391_open (name))) {
        /* This file does not exist. We need to create a new file. */

        /* Code that calls an ioctl that creates a new file. */
        ece391_ioctl(-1, 0, name);

        fd = ece391_open(name);
    }   

    if (-1 == ece391_write (fd, buf, ece391_strlen(buf))) {
    	ece391_fdputs (1, (uint8_t*)"file write failed\n");
	    return 3;
    }
    
	
    return 0;
}