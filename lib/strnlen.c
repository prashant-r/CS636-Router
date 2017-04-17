/* strnlen.c - strnlen */
#include <xinu.h>
/*------------------------------------------------------------------------
 *  strnlen  -  Return the length of a string (number of non-NULL bytes),
 *			up to the max specified by n
 *------------------------------------------------------------------------
 */
int	strnlen(
	  const char	*s,		/* string			*/
	  unsigned int	len		/* max length			*/
		)
{
    int n;

    n = 0;
    while (*s++ && n < len)
        n++;

    return (n);
}


long strtol(const char *str, char **endptr, int base)
{
    int c, neg = 0;
    const char *alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
    long val = 0;

    if (base != 0 && (base < 2 || base > 36)) return 0;

    /* skip whitespace */
    while (*str && isspace(*str))
        str++;

    /* parse potential -/+ (default to positive) */
    if (*str == '-')
    {
        neg = 1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    /* parse prefix (0x, 0) */
    if (base == 0 || base == 16)
    {
        if (str[0] == '0' && str[1] == 'x')
        {
            base = 16;
            str += 2;
        }
    }
    if (base == 0)
    {
        if (str[0] == '0')
            base = 8;
    }
    if (base == 0)
        base = 10;

    /* parse number */
    while ((c = *str))
    {
        /* search for c */
        const char *idx = memchr(alphabet, tolower(c), base);
        if (idx == NULL)
            break;
        /* XXX clamp to LONG_MAX / LONG_MIN */
        val = val * base + (idx - alphabet);
        str++;
    }

    if (endptr)
        *endptr = (char *)str;
    if (neg)
        return -val;
    else
        return val;
}

unsigned long strtoul(const char *str, char **endptr, int base)
{
    return (unsigned long)strtol(str, endptr, base);
}