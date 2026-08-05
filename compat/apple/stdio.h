#ifndef STDIO_COMPAT_H
#define STDIO_COMPAT_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "types.h"

// Recreate the GLIBC types and structure for macOS
typedef ssize_t (*cookie_read_function_t)(void *cookie, char *buf, size_t size);
typedef ssize_t (*cookie_write_function_t)(void *cookie, const char *buf, size_t size);
typedef int (*cookie_seek_function_t)(void *cookie, off64_t *offset, int whence);
typedef int (*cookie_close_function_t)(void *cookie);

typedef struct {
    cookie_read_function_t  read;
    cookie_write_function_t write;
    cookie_seek_function_t  seek;
    cookie_close_function_t close;
} cookie_io_functions_t;

// Internal wrapper to hold the user's cookie and the function pointers
typedef struct {
    void *user_cookie;
    cookie_io_functions_t hooks;
} shim_context_t;

// Trampoline functions to adapt BSD funopen signatures to GLIBC fopencookie
static __inline
int shim_read_fn(void *cookie, char *buf, int size) {
    shim_context_t *ctx = (shim_context_t *)cookie;
    if (!ctx->hooks.read) {
        return 0; // EOF
    }
    if (size < 0) {
        errno = EINVAL;
        return -1;
    }
    return (int)ctx->hooks.read(ctx->user_cookie, buf, (size_t)size);
}

static __inline
int shim_write_fn(void *cookie, const char *buf, int size) {
    shim_context_t *ctx = (shim_context_t *)cookie;
    if (!ctx->hooks.write) {
        return size; // Mimic successful write if no hook provided
    }
    if (size < 0) {
        errno = EINVAL;
        return -1;
    }
    return (int)ctx->hooks.write(ctx->user_cookie, buf, (size_t)size);
}

static __inline
fpos_t shim_seek_fn(void *cookie, fpos_t offset, int whence) {
    shim_context_t *ctx = (shim_context_t *)cookie;
    if (!ctx->hooks.seek) {
        errno = ESPIPE; // Illegal seek
        return -1;
    }
    
    off64_t offset64 = (off64_t)offset;
    int result = ctx->hooks.seek(ctx->user_cookie, &offset64, whence);
    
    if (result < 0) {
        return -1;
    }
    return (fpos_t)offset64;
}

static __inline
int shim_close_fn(void *cookie) {
    shim_context_t *ctx = (shim_context_t *)cookie;
    int result = 0;
    
    if (ctx->hooks.close) {
        result = ctx->hooks.close(ctx->user_cookie);
    }
    
    // Crucial: Free the wrapper context allocation when fclose() is called
    free(ctx);
    return result;
}

// The main fopencookie emulation function
__inline
FILE *fopencookie(void *cookie, const char *mode, cookie_io_functions_t io_funcs) {
    shim_context_t *ctx = malloc(sizeof(shim_context_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->user_cookie = cookie;
    ctx->hooks = io_funcs;
    
    // Map hooks to funopen. macOS passes NULL if the function pointer is absent.
    FILE *stream = funopen(
        ctx,
        io_funcs.read  ? shim_read_fn  : NULL,
        io_funcs.write ? shim_write_fn : NULL,
        io_funcs.seek  ? shim_seek_fn  : NULL,
        shim_close_fn // Always provide close to free our internal ctx structure
    );
    
    if (!stream) {
        free(ctx);
        return NULL;
    }
    
    return stream;
}

#endif /* STDIO_COMPAT_H */

