
#include "lib391/ece391syscall.h"
#include "lib391/ece391support.h"
#include "lib391/ece391sysnum.h"

int main () {

    /* The format will be echo1 [filename] <(<) arguments (different from that of the usual linux syntax) */
    int32_t fd, cnt;
    uint8_t name [1024];
    ece391_memset(name, 0, 1024);
    int filler = 0;
    ece391_strcpy(name , (const uint8_t * )"new.txt"); /* Hard coded for now*/
    uint8_t args[1024];
    ece391_memset(args, 0, 1024);

    int filler2 = 0;
    uint8_t buf[1024];
    ece391_memset(buf, 0, 1024);
 
    int filler3 = 0;
    int res;

    if (0 != (res = ece391_getargs (buf, 1024) ) ) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }

    int name_counter = 0;
    int arg_counter = 0;
    while (buf[name_counter] != ' ') {
        if (buf[name_counter ] == '\0') {
            return 3;
        }
        name[name_counter] = buf[name_counter];
        name_counter ++;
    }
    name[name_counter ++] = 0;
    
    while(buf[name_counter] != 0) {
        args[arg_counter++] = buf[name_counter];
        name_counter ++;
    }
    args[arg_counter++] = 0;

    if ( -1 == (fd = ece391_open(name))) {
        ece391_ioctl(-1, 0, name); /* Create a new blank file*/
        fd = ece391_open(name);
    } else {
        ece391_ioctl(-1, 1, name); /* Clear the file */
    }
    
    if ( ece391_write(fd, args,ece391_strlen(args) ) == 0) return 3; else return 0;

    ece391_close(fd);
	return 0;
    

}