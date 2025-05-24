#include <string.h>

char *strtok_r(char *s, const char *delim, char **last) {
    char *spanp;
    int c;
    int sc;
    char *tok;

    if (s == 0 && (s = *last) == 0)
        return 0;

cont:
    c = *s++;
    for (spanp = (char *) delim; (sc = *spanp++) != 0; ) {
        if (c == sc)
            goto cont;
    }
    if (c == 0) {
        *last = 0;
        return 0;
    }
    tok = s - 1;

    for (;;) {
        c = *s++;
        spanp = (char *) delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = 0;
                else
                    s[-1] = 0;
                *last = s;
                return tok;
            }
        } while (sc != 0);
    }
}

char *strtok(char *s, const char *delim) {
    static char *last;

    return strtok_r(s, delim, &last);
}
