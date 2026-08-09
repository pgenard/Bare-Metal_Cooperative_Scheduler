#ifndef STRING_H
#define STRING_H

unsigned int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char*, const char*, unsigned int);
unsigned long strtoul_hex(const char*);
unsigned int strtoul_dec(const char*);

#endif
