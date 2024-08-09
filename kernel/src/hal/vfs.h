#pragma once

#include <stdint.h>
#include <stddef.h>

#define VFS_FD_STDIN    0
#define VFS_FD_STDOUT   1
#define VFS_FD_STDERR   2
#define VFS_FD_DEBUG    3

typedef uint32_t fd_t;

size_t vfs_write(fd_t fd, uint8_t *data, size_t size);
