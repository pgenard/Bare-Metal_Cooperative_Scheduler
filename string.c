#include "string.h"

unsigned int strlen(const char *s)
{
    unsigned int n = 0;

    while (*s++)
        n++;

    return n;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, unsigned int n)
{
    while (n--) {
        unsigned char c1 = *s1++;
        unsigned char c2 = *s2++;

        if (c1 != c2)
            return c1 - c2;

        if (c1 == '\0')
            return 0;
    }

    return 0;
}

unsigned long strtoul_hex(const char *s)
{
    unsigned long value = 0;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        s += 2;

    while (*s)
    {
        unsigned int digit;

        if (*s >= '0' && *s <= '9')
            digit = *s - '0';
        else if (*s >= 'a' && *s <= 'f')
            digit = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F')
            digit = *s - 'A' + 10;
        else
            break;

        value <<= 4;
        value |= digit;

        s++;
    }

    return value;
}

unsigned int strtoul_dec(const char *s) {
    unsigned int value = 0;

    while (*s >= '0' && *s <= '9')
    {
        value = value * 10 + (*s - '0');
        s++;
    }

    return value;
}
