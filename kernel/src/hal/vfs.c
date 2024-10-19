#include "vfs.h"
#include "e9.h"
#include <drivers/fb.h>
#include <graphics/graphics.h>
#include <util/logger.h>

size_t vfs_write(fd_t fd, uint8_t *data, size_t size) {
    switch (fd) {
        case VFS_FD_STDIN:
            return 0;
        case VFS_FD_STDOUT:
        case VFS_FD_STDERR:
            if (!fb_ready())
                return size;

            for (size_t i = 0; i < size; i++)
                fb_putc(data[i]);
            return size;
        case VFS_FD_DEBUG:
#ifdef DEBUG_MODE
            for (size_t i = 0; i < size; i++)
                e9_putc(data[i]);
#endif
            return size;

    }
    return 0;
}
