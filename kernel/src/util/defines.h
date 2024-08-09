#pragma once

#define ARCH x86_64
#define OS_VERSION 0.0.1

#define STRINGIFY_DIR(x) #x
#define STRINGIFY(x) STRINGIFY_DIR(x)

#define _cdecl  __attribute__((cdecl))
#define _unused __attribute__((unused))
#define _packed __attribute__((packed))
