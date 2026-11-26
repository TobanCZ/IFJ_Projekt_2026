#include <stdlib.h>
#include <string.h>

// Pomocná funkce pro kopírování řetězce
static char *copy_str(const char *s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}