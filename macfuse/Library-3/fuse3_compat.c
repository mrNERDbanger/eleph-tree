/*
 * FUSE API version 3 compatibility layer implementation
 * This provides FUSE v3 API support on top of macFUSE v2
 */

#include "fuse3.h"
#undef FUSE_MAJOR_VERSION
#undef FUSE_MINOR_VERSION
#include <fuse/fuse.h>  // macFUSE v2 API
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>

/* Debug logging */
#ifdef FUSE3_DEBUG
#define fuse3_debug(fmt, ...) fprintf(stderr, "FUSE3_COMPAT: " fmt "\n", ##__VA_ARGS__)
#else
#define fuse3_debug(fmt, ...)
#endif

#define fuse3_error(fmt, ...) do { \
    fprintf(stderr, "FUSE3_COMPAT ERROR: " fmt "\n", ##__VA_ARGS__); \
    syslog(LOG_ERR, "FUSE3_COMPAT ERROR: " fmt, ##__VA_ARGS__); \
} while(0)

/* Internal mapping structure */
struct fuse3_internal {
    struct fuse *fuse2_handle;
    const struct fuse3_operations *ops3;
    void *user_data;
};

/* Convert FUSE v2 file_info to v3 */
static void convert_file_info_2_to_3(const struct fuse_file_info *fi2, struct fuse3_file_info *fi3) {
    if (!fi2 || !fi3) {
        fuse3_error("Invalid file_info pointer in conversion");
        return;
    }
    
    memset(fi3, 0, sizeof(*fi3));
    fi3->flags = fi2->flags;
    fi3->fh = fi2->fh;
    fi3->direct_io = fi2->direct_io;
    fi3->keep_cache = fi2->keep_cache;
    fi3->flush = fi2->flush;
    fi3->nonseekable = fi2->nonseekable;
    fi3->lock_owner = fi2->lock_owner;
    fuse3_debug("Converted file_info v2->v3: fh=%llu, flags=0x%x", fi3->fh, fi3->flags);
}

/* Convert FUSE v3 file_info to v2 */
static void convert_file_info_3_to_2(const struct fuse3_file_info *fi3, struct fuse_file_info *fi2) {
    if (!fi3 || !fi2) {
        fuse3_error("Invalid file_info pointer in conversion");
        return;
    }
    
    memset(fi2, 0, sizeof(*fi2));
    fi2->flags = fi3->flags;
    fi2->fh = fi3->fh;
    fi2->direct_io = fi3->direct_io;
    fi2->keep_cache = fi3->keep_cache;
    fi2->flush = fi3->flush;
    fi2->nonseekable = fi3->nonseekable;
    fi2->lock_owner = fi3->lock_owner;
    fuse3_debug("Converted file_info v3->v2: fh=%llu, flags=0x%x", fi2->fh, fi2->flags);
}

/* Wrapper functions that convert between FUSE v2 and v3 APIs */

static int fuse3_getattr_wrapper(const char *path, struct stat *stbuf) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (!internal) {
        fuse3_error("No internal context in getattr for path: %s", path);
        return -EINVAL;
    }
    if (internal->ops3->getattr) {
        fuse3_debug("getattr called for path: %s", path);
        int ret = internal->ops3->getattr(path, stbuf, NULL);
        if (ret < 0) {
            fuse3_debug("getattr failed for path %s: %s", path, strerror(-ret));
        }
        return ret;
    }
    return -ENOSYS;
}

static int fuse3_readlink_wrapper(const char *path, char *buf, size_t size) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->readlink) {
        return internal->ops3->readlink(path, buf, size);
    }
    return -ENOSYS;
}

static int fuse3_mknod_wrapper(const char *path, mode_t mode, dev_t rdev) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->mknod) {
        return internal->ops3->mknod(path, mode, rdev);
    }
    return -ENOSYS;
}

static int fuse3_mkdir_wrapper(const char *path, mode_t mode) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->mkdir) {
        return internal->ops3->mkdir(path, mode);
    }
    return -ENOSYS;
}

static int fuse3_unlink_wrapper(const char *path) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->unlink) {
        return internal->ops3->unlink(path);
    }
    return -ENOSYS;
}

static int fuse3_rmdir_wrapper(const char *path) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->rmdir) {
        return internal->ops3->rmdir(path);
    }
    return -ENOSYS;
}

static int fuse3_open_wrapper(const char *path, struct fuse_file_info *fi) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->open) {
        struct fuse3_file_info fi3;
        convert_file_info_2_to_3(fi, &fi3);
        int ret = internal->ops3->open(path, &fi3);
        convert_file_info_3_to_2(&fi3, fi);
        return ret;
    }
    return -ENOSYS;
}

static int fuse3_read_wrapper(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->read) {
        struct fuse3_file_info fi3;
        convert_file_info_2_to_3(fi, &fi3);
        return internal->ops3->read(path, buf, size, offset, &fi3);
    }
    return -ENOSYS;
}

static int fuse3_write_wrapper(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->write) {
        struct fuse3_file_info fi3;
        convert_file_info_2_to_3(fi, &fi3);
        return internal->ops3->write(path, buf, size, offset, &fi3);
    }
    return -ENOSYS;
}

static int fuse3_release_wrapper(const char *path, struct fuse_file_info *fi) {
    struct fuse3_internal *internal = fuse_get_context()->private_data;
    if (internal->ops3->release) {
        struct fuse3_file_info fi3;
        convert_file_info_2_to_3(fi, &fi3);
        return internal->ops3->release(path, &fi3);
    }
    return -ENOSYS;
}

/* FUSE v3 API implementation */

struct fuse3 *fuse3_new(struct fuse3_args *args, const struct fuse3_operations *op, size_t op_size __attribute__((unused)), void *private_data) {
    if (!args || !op) {
        fuse3_error("Invalid arguments to fuse3_new");
        return NULL;
    }
    
    openlog("fuse3_compat", LOG_PID | LOG_CONS, LOG_USER);
    fuse3_debug("Initializing FUSE3 compatibility layer");
    
    struct fuse3_internal *internal = malloc(sizeof(struct fuse3_internal));
    if (!internal) {
        fuse3_error("Failed to allocate memory for internal structure");
        return NULL;
    }
    
    internal->ops3 = op;
    internal->user_data = private_data;
    
    /* Create FUSE v2 operations structure */
    struct fuse_operations ops2;
    memset(&ops2, 0, sizeof(ops2));
    
    /* Map FUSE v3 operations to v2 equivalents */
    if (op->getattr) ops2.getattr = fuse3_getattr_wrapper;
    if (op->readlink) ops2.readlink = fuse3_readlink_wrapper;
    if (op->mknod) ops2.mknod = fuse3_mknod_wrapper;
    if (op->mkdir) ops2.mkdir = fuse3_mkdir_wrapper;
    if (op->unlink) ops2.unlink = fuse3_unlink_wrapper;
    if (op->rmdir) ops2.rmdir = fuse3_rmdir_wrapper;
    if (op->open) ops2.open = fuse3_open_wrapper;
    if (op->read) ops2.read = fuse3_read_wrapper;
    if (op->write) ops2.write = fuse3_write_wrapper;
    if (op->release) ops2.release = fuse3_release_wrapper;
    
    /* Convert args structure */
    struct fuse_args args2 = { args->argc, args->argv, args->allocated };
    
    /* Extract mount point from arguments */
    char *mountpoint = NULL;
    if (args->argc > 1) {
        mountpoint = args->argv[args->argc - 1];
    }
    if (!mountpoint) {
        fuse3_error("No mount point specified");
        free(internal);
        return NULL;
    }
    
    fuse3_debug("Mounting filesystem at: %s", mountpoint);
    
    /* Create FUSE channel first (required for macFUSE) */
    struct fuse_chan *ch = fuse_mount(mountpoint, &args2);
    if (!ch) {
        fuse3_error("Failed to mount filesystem at %s: %s", mountpoint, strerror(errno));
        free(internal);
        return NULL;
    }
    
    /* Create FUSE v2 handle */
    internal->fuse2_handle = fuse_new(ch, &args2, &ops2, sizeof(ops2), internal);
    if (!internal->fuse2_handle) {
        fuse3_error("Failed to create FUSE handle: %s", strerror(errno));
        fuse_unmount(mountpoint, ch);
        free(internal);
        return NULL;
    }
    
    return (struct fuse3 *)internal;
}

int fuse3_mount(struct fuse3 *f, const char *mountpoint) {
    /* Mount functionality is handled during fuse3_new() in macFUSE */
    (void)f;
    (void)mountpoint;
    return 0;
}

void fuse3_unmount(struct fuse3 *f) {
    /* Unmount will be handled when destroying the handle */
    (void)f;
}

int fuse3_loop(struct fuse3 *f) {
    struct fuse3_internal *internal = (struct fuse3_internal *)f;
    if (!internal || !internal->fuse2_handle) {
        fuse3_error("Invalid handle passed to fuse3_loop");
        return -1;
    }
    
    fuse3_debug("Starting FUSE event loop");
    int ret = fuse_loop(internal->fuse2_handle);
    if (ret < 0) {
        fuse3_error("FUSE loop failed: %s", strerror(-ret));
    }
    return ret;
}

void fuse3_destroy(struct fuse3 *f) {
    struct fuse3_internal *internal = (struct fuse3_internal *)f;
    if (!internal) return;
    
    fuse3_debug("Destroying FUSE3 handle");
    
    if (internal->fuse2_handle) {
        fuse_destroy(internal->fuse2_handle);
    }
    free(internal);
    closelog();
}

struct fuse3_session *fuse3_get_session(struct fuse3 *f) {
    /* Return a dummy session for compatibility */
    return (struct fuse3_session *)f;
}

int fuse3_session_loop(struct fuse3_session *se) {
    /* Use fuse3_loop instead */
    return fuse3_loop((struct fuse3 *)se);
}

int fuse3_parse_cmdline(struct fuse3_args *args, struct fuse3_cmdline_opts *opts) {
    /* Convert to FUSE v2 args and parse */
    struct fuse_args args2 = { args->argc, args->argv, args->allocated };
    
    char *mountpoint = NULL;
    int multithreaded = 0;
    int foreground = 0;
    
    int ret = fuse_parse_cmdline(&args2, &mountpoint, &multithreaded, &foreground);
    
    /* Update original args */
    args->argc = args2.argc;
    args->argv = args2.argv;
    args->allocated = args2.allocated;
    
    if (opts && ret == 0) {
        memset(opts, 0, sizeof(*opts));
        /* Set basic options - extend as needed */
    }
    
    if (mountpoint) {
        free(mountpoint);
    }
    
    return ret;
}