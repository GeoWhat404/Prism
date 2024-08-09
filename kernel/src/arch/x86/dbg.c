#include <stdint.h>
#include <util/debug.h>
#include <util/defines.h>

_cdecl void dbg_trap() {
    log_debug(MODULE_INTRPT, "Debug trap called");
}

_cdecl void handle_tssrsp(uint64_t rsp) {
    log_debug(MODULE_INTRPT, "handle_tssrsp");
}

_cdecl void syscall_handler() {
    log_debug(MODULE_INTRPT, "syscall_handler");
}

_cdecl void handle_syscall_tssrsp() {
    log_debug(MODULE_INTRPT, "handle_syscall_tssrsp");
}

_cdecl void task_kill_cleanup() {
    log_debug(MODULE_INTRPT, "task_kill_cleanup");
}
