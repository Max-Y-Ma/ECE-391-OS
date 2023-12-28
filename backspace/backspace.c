
#include "lib391/ece391syscall.h"
#include "lib391/ece391support.h"
#include "lib391/ece391sysnum.h"

uint32_t power_ten( int pow);

int32_t string_to_int(uint8_t * args );

int main() {
    int32_t fd, cnt;
    uint8_t args [1024];
    ece391_memset(args, 0, 1024);
    uint8_t name [1024];
    ece391_memset(name,0,1024);
    uint8_t buf[1024];
    ece391_memset(buf,0,1024);
    
    if ( -1 == ece391_getargs(buf, 1024) ) {
        return 3;
    }
    int name_counter = 0;
    int arg_counter =0;
    while (buf[name_counter] != ' ') {
        if (buf[name_counter ] == '\0') {
            return 3;
        }
        name[name_counter] = buf[name_counter];
        name_counter ++;
    }
    name[name_counter ++] = 0;

    while(buf[name_counter] != 0) {
        if (buf[name_counter] == ' ') continue;
        args[arg_counter++] = buf[name_counter];
        name_counter ++;
    }


    args[arg_counter++] = 0;
    int len = 1;


     if ( (fd = ece391_open(name) ) != -1 )
    {
    ece391_ioctl(-1, 2, fd); /* File exists. Update file position*/
    ece391_ioctl(-1,3,fd); /* Set to Delete Mode */
    } else {
        ece391_ioctl(-1,0, name); /* File doesn't exist. Update file position*/
        fd = ece391_open(name);
    }
    if (string_to_int(args) == -1) {
        return 3;
    }
    if (0 == ece391_write(fd, (uint8_t * )"\0",string_to_int(args ) )) return 3;

    ece391_close(fd);
    return 0;
}

uint32_t power_ten(int pow) {
    if (pow == 0 ) {
        return 1;
    }
    int i;
    int total = 1;
    for (i = 1; i <= pow; i++) {
        total *= 10;
    }

    return total;
}

int32_t string_to_int(uint8_t * args) {
    int len = ece391_strlen(args);
    int i;
    int total = 0;
    for (i = 0 ; i < len ; i ++ ){ 
        if (args[i] < 48 || args[i] > 57) {
            return -1;
        }
        total +=   (args[i] - 48) * power_ten(--len); 
    }
    return total;
}




