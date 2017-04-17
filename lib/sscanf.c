/* sscanf.c - sscanf, sgetch, sungetch */

#define EOF   (-2)
#define NULL  0

static int sgetch(int, char **);
static int sungetch(int, char **);
extern int _doscan(register char *, register int **,
                   int (*getc) (int, char **),
                   int (*ungetc) (int, char **), int, int);
extern int strcspn(char *string, char *chars);
int strcspn(char *string, char *chars)
{
    register char c, *p, *s;

    for (s = string, c = *s; c != 0; s++, c = *s) {
    for (p = chars; *p != 0; p++) {
        if (c == *p) {
        return s-string;
        }
    }
    }
    return s-string;
}

char * strtok(char *s , const char *delim)
{
    static char *lasts;
    register int ch;

    if (s == 0)
    s = lasts;
    do {
    if ((ch = *s++) == '\0')
        return 0;
    } while (strchr(delim, ch));
    --s;
    lasts = s + strcspn(s, delim);
    if (*lasts != 0)
    *lasts++ = 0;
    return s;
}



/*------------------------------------------------------------------------
 *  sscanf  -  Read from a string according to a format.
 *			   Return result of _doscan.
 *------------------------------------------------------------------------
 */
int	sscanf(
	  char		*str,		/* String to read from		*/
	  char		*fmt,		/* Format string		*/
	  int		args		/* Num of args in format string	*/
	)
{
    char *s;

    s = str;
    return (_doscan(fmt, (int **)&args, sgetch, sungetch, 0, (int)&s));
}

/*------------------------------------------------------------------------
 *  sgetch  -  Get the next character from a string
 *------------------------------------------------------------------------
 */
static int	sgetch(
		  int		dummy,	/* Dummy unused variable	*/
		  char		**cpp	/* String to use		*/
		)
{
    return (*(*cpp) == '\0' ? EOF : *(*cpp)++);
}

/*------------------------------------------------------------------------
 *  sungetch  -  Pushback a character in a string
 *------------------------------------------------------------------------
 */
static int	sungetch(
		  int		dummy,	/* Dummy unused variable	*/
		  char		**cpp	/* String for pushback		*/
		)
{
    return (*(*cpp)--);
}
