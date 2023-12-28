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

int32_t ece391_atoi(const char *str) {
    int result = 0;
    int sign = 1; // Positive by default

    // Skip leading whitespaces
    while (*str == ' ' || *str == '\t' || *str == '\n')
        str++;

    // Check for the sign
    if (*str == '-' || *str == '+') {
        sign = (*str == '-') ? -1 : 1;
        str++;
    }

    // Process digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}
