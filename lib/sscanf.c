/* sscanf.c - sscanf, sgetch, sungetch */

#define EOF   (-2)
#define NULL  0

static int sgetch(int, char **);
static int sungetch(int, char **);
extern int _doscan(register char *, register int **,
                   int (*getc) (int, char **),
                   int (*ungetc) (int, char **), int, int);
extern char * strtok(s, delim);

char * strtok(s, delim)
    register char *s;
    register const char *delim;
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;


    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {       /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
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
