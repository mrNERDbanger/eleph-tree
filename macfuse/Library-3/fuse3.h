#ifndef FUSE3_H
#define FUSE3_H

/*
 * FUSE API version 3 compatibility layer for macFUSE
 * This provides FUSE v3 API support on top of macFUSE
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/file.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* FUSE version 3 major version */
#define FUSE_MAJOR_VERSION 3
#define FUSE_MINOR_VERSION 0

/* Use FUSE API version 30 (3.0) */
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 30
#endif

/* Forward declarations */
struct fuse3;
struct fuse3_session;
struct fuse3_pollhandle;

/* Enums and typedefs */
enum fuse3_readdir_flags {
    FUSE3_READDIR_PLUS = (1 << 0),
};

enum fuse3_fill_dir_flags {
    FUSE3_FILL_DIR_PLUS = (1 << 1),
};

enum fuse3_buf_flags {
    FUSE3_BUF_IS_FD = (1 << 1),
    FUSE3_BUF_FD_SEEK = (1 << 2),
    FUSE3_BUF_FD_RETRY = (1 << 3),
};

/* File information structure for FUSE v3 */
struct fuse3_file_info {
    int flags;
    unsigned long fh_old;
    int writepage;
    unsigned int direct_io : 1;
    unsigned int keep_cache : 1;
    unsigned int flush : 1;
    unsigned int nonseekable : 1;
    unsigned int flock_release : 1;
    unsigned int cache_readdir : 1;
    unsigned int padding : 26;
    uint64_t fh;
    uint64_t lock_owner;
    uint32_t poll_events;
};

/* Command line options structure */
struct fuse3_cmdline_opts {
    int foreground;
    int debug;
    int nodefault_subtype;
    char *mountpoint;
    int show_version;
    int show_help;
    int clone_fd;
    unsigned int max_idle_threads;
};

/* Connection information structure for FUSE v3 */
struct fuse3_conn_info {
    unsigned proto_major;
    unsigned proto_minor;
    unsigned max_write;
    unsigned max_read;
    unsigned max_readahead;
    unsigned capable;
    unsigned want;
    unsigned max_background;
    unsigned congestion_threshold;
    unsigned time_gran;
    unsigned reserved[22];
};

/* Configuration structure for FUSE v3 */
struct fuse3_config {
    int set_gid;
    unsigned int gid;
    int set_uid;
    unsigned int uid;
    int set_mode;
    unsigned int umask;
    double entry_timeout;
    double negative_timeout;
    double attr_timeout;
    int intr;
    int intr_signal;
    int remember;
    int hard_remove;
    int use_ino;
    int readdir_ino;
    int direct_io;
    int kernel_cache;
    int auto_cache;
    int ac_attr_timeout_set;
    double ac_attr_timeout;
    int nullpath_ok;
    int show_help;
    char *modules;
    int debug;
};

/* Arguments structure */
struct fuse3_args {
    int argc;
    char **argv;
    int allocated;
};

/* Buffer structures for efficient I/O */
struct fuse3_buf {
    size_t size;
    enum fuse3_buf_flags flags;
    void *mem;
    int fd;
    off_t pos;
};

struct fuse3_bufvec {
    size_t count;
    size_t idx;
    size_t off;
    struct fuse3_buf buf[1];
};

/* Utility function typedef */
typedef int (*fuse3_fill_dir_t)(void *buf, const char *name, const struct stat *stbuf, off_t off, enum fuse3_fill_dir_flags flags);

/* FUSE v3 operations structure */
struct fuse3_operations {
    int (*getattr)(const char *path, struct stat *stbuf, struct fuse3_file_info *fi);
    int (*readlink)(const char *path, char *buf, size_t size);
    int (*mknod)(const char *path, mode_t mode, dev_t rdev);
    int (*mkdir)(const char *path, mode_t mode);
    int (*unlink)(const char *path);
    int (*rmdir)(const char *path);
    int (*symlink)(const char *from, const char *to);
    int (*rename)(const char *from, const char *to, unsigned int flags);
    int (*link)(const char *from, const char *to);
    int (*chmod)(const char *path, mode_t mode, struct fuse3_file_info *fi);
    int (*chown)(const char *path, uid_t uid, gid_t gid, struct fuse3_file_info *fi);
    int (*truncate)(const char *path, off_t size, struct fuse3_file_info *fi);
    int (*open)(const char *path, struct fuse3_file_info *fi);
    int (*read)(const char *path, char *buf, size_t size, off_t offset, struct fuse3_file_info *fi);
    int (*write)(const char *path, const char *buf, size_t size, off_t offset, struct fuse3_file_info *fi);
    int (*statfs)(const char *path, struct statvfs *stbuf);
    int (*flush)(const char *path, struct fuse3_file_info *fi);
    int (*release)(const char *path, struct fuse3_file_info *fi);
    int (*fsync)(const char *path, int isdatasync, struct fuse3_file_info *fi);
    int (*setxattr)(const char *path, const char *name, const char *value, size_t size, int flags);
    int (*getxattr)(const char *path, const char *name, char *value, size_t size);
    int (*listxattr)(const char *path, char *list, size_t size);
    int (*removexattr)(const char *path, const char *name);
    int (*opendir)(const char *path, struct fuse3_file_info *fi);
    int (*readdir)(const char *path, void *buf, fuse3_fill_dir_t filler, off_t offset, struct fuse3_file_info *fi, enum fuse3_readdir_flags flags);
    int (*releasedir)(const char *path, struct fuse3_file_info *fi);
    int (*fsyncdir)(const char *path, int isdatasync, struct fuse3_file_info *fi);
    void *(*init)(struct fuse3_conn_info *conn, struct fuse3_config *cfg);
    void (*destroy)(void *private_data);
    int (*access)(const char *path, int mask);
    int (*create)(const char *path, mode_t mode, struct fuse3_file_info *fi);
    int (*lock)(const char *path, struct fuse3_file_info *fi, int cmd, struct flock *lock);
    int (*utimens)(const char *path, const struct timespec tv[2], struct fuse3_file_info *fi);
    int (*bmap)(const char *path, size_t blocksize, uint64_t *idx);
    int (*ioctl)(const char *path, int cmd, void *arg, struct fuse3_file_info *fi, unsigned int flags, void *data);
    int (*poll)(const char *path, struct fuse3_file_info *fi, struct fuse3_pollhandle *ph, unsigned *reventsp);
    int (*write_buf)(const char *path, struct fuse3_bufvec *buf, off_t offset, struct fuse3_file_info *fi);
    int (*read_buf)(const char *path, struct fuse3_bufvec **bufp, size_t size, off_t offset, struct fuse3_file_info *fi);
    int (*flock)(const char *path, struct fuse3_file_info *fi, int op);
    int (*fallocate)(const char *path, int mode, off_t offset, off_t length, struct fuse3_file_info *fi);
    ssize_t (*copy_file_range)(const char *path_in, struct fuse3_file_info *fi_in, off_t offset_in, const char *path_out, struct fuse3_file_info *fi_out, off_t offset_out, size_t size, int flags);
    off_t (*lseek)(const char *path, off_t off, int whence, struct fuse3_file_info *fi);
};

/* FUSE v3 API functions */
struct fuse3 *fuse3_new(struct fuse3_args *args, const struct fuse3_operations *op, size_t op_size, void *private_data);
int fuse3_mount(struct fuse3 *f, const char *mountpoint);
void fuse3_unmount(struct fuse3 *f);
int fuse3_loop(struct fuse3 *f);
void fuse3_destroy(struct fuse3 *f);

/* Session management */
struct fuse3_session *fuse3_get_session(struct fuse3 *f);
int fuse3_session_loop(struct fuse3_session *se);

/* Command line parsing */
int fuse3_parse_cmdline(struct fuse3_args *args, struct fuse3_cmdline_opts *opts);

#ifdef __cplusplus
}
#endif

#endif /* FUSE3_H */