
#include "lib391/ece391syscall.h"
#include "lib391/ece391support.h"
#include "lib391/ece391sysnum.h"


/* Append to a file, not overwrite it.*/
int main() {
    int32_t fd, cnt;
    uint8_t name[1024];
    ece391_memset(name,0,1024);
    uint8_t args[1024];
    ece391_memset(args,0,1024);
    uint8_t buf[1024];
    ece391_memset(buf,0,1024);

    if ( -1 == ece391_getargs(buf, 1024) ) {
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
    if ( (fd = ece391_open(name) ) != -1 )
    {
    ece391_ioctl(-1, 2, fd); /* File exists. Update file position*/
    } else {
        ece391_ioctl(-1,0, name); /* File doesn't exist. Update file position*/
        fd = ece391_open(name);
    }
    
    if (0 == ece391_write(fd, args, ece391_strlen(args)) ) return 3;

    ece391_close(fd);
    return 0;
}