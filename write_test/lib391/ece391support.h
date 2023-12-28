#ifndef _ECE391SUPPORT_H_
#define _ECE391SUPPORT_H_

#include "types.h"

#define TERMINAL_BUFFER_LENGTH 129

uint32_t ece391_strlen (const uint8_t* s);
void ece391_strcpy (uint8_t* dst, const uint8_t* src);
void ece391_fdputs (int32_t fd, const uint8_t* s);
int32_t ece391_strcmp (const uint8_t* s1, const uint8_t* s2);
int32_t ece391_strncmp (const uint8_t* s1, const uint8_t* s2, uint32_t n);
uint8_t ece391_getc (void);

#endif /* _ECE391SUPPORT_H_ */
