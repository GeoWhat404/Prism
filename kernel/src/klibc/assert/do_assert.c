#include <assert.h>

void do_assert(bool expr, const struct _source_location loc, const char *expression) {
    if (!expr) {
        panic("assertion %s\nfailed at %s in %s:%d", expression, loc.function_name, loc.file_name, loc.line_number);
    }
}
