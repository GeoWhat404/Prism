/* Temporary declarations to satisfy nasm */

#include <stdint.h>
#include <util/defines.h>

_cdecl void handle_tssrsp(uint64_t rsp) {
}

_cdecl void syscall_handler(void) {
}

_cdecl void handle_syscall_tssrsp(void) {
}

_cdecl void task_kill_cleanup(void) {
}
