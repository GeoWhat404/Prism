#include <string.h>

void strrev(char *s) {
    if (s == 0)
        return;

    int i = 0;
    int j = strlen(s) + 1;

    while (i < j) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
        i++;
        j--;
    }
}
