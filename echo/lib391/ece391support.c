#include "ece391support.h"
#include "ece391syscall.h"

uint32_t ece391_strlen (const uint8_t* s)
{
    uint32_t len;

    for (len = 0; '\0' != *s; s++, len++);
    return len;
}

void ece391_strcpy (uint8_t* dst, const uint8_t* src)
{
    while ('\0' != (*dst++ = *src++));
}

void ece391_fdputs (int32_t fd, const uint8_t* s)
{
    (void)ece391_write (fd, s, ece391_strlen (s));
}

int32_t ece391_strcmp (const uint8_t* s1, const uint8_t* s2)
{
    while (*s1 == *s2) {
        if (*s1 == '\0')
	    return 0;
	s1++;
	s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

int32_t ece391_strncmp (const uint8_t* s1, const uint8_t* s2, uint32_t n)
{
    if (0 == n)
	return 0;
    while (*s1 == *s2) {
        if (*s1 == '\0' || --n == 0)
	    return 0;
	s1++;
	s2++;
    }
    return ((int32_t)*s1) - ((int32_t)*s2);
}

uint8_t ece391_getc (void)
{
    /* Call read system call */
    uint8_t terminal_buf[TERMINAL_BUFFER_LENGTH];
    if (ece391_read(STDIN, terminal_buf, 2) == -1) {
        return -1;
    }

    /* Return first typed character */
    return terminal_buf[0];
}
int8_t* ece391_strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

void* ece391_memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}