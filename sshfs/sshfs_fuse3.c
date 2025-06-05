/*
 * SSHFS - Secure Shell File System with FUSE v3 API
 * Modified to use FUSE3 compatibility layer for macOS
 * 
 * Based on the original SSHFS by Miklos Szeredi
 * FUSE3 adaptation using eleph-tree compatibility layer
 */

#define FUSE_USE_VERSION 30

#include <fuse3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <libssh2.h>
#include <libssh2_sftp.h>

/* SSHFS configuration and state */
struct sshfs {
    char *host;
    char *username;
    char *password;
    int port;
    
    /* SSH/SFTP session handles */
    LIBSSH2_SESSION *session;
    LIBSSH2_SFTP *sftp;
    int sock;
    
    /* Threading */
    pthread_mutex_t lock;
    
    /* Options */
    int reconnect;
    int follow_symlinks;
    int no_check_root;
    int debug;
};

static struct sshfs sshfs = {
    .port = 22,
    .reconnect = 1,
    .follow_symlinks = 0,
    .no_check_root = 0,
    .debug = 0,
};

/* Helper functions */
static void sshfs_log(const char *fmt, ...)
{
    if (sshfs.debug) {
        va_list ap;
        va_start(ap, fmt);
        fprintf(stderr, "SSHFS_FUSE3: ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        va_end(ap);
    }
}

/* Simplified SSH connection (for demo purposes) */
static int sshfs_connect(void)
{
    sshfs_log("Connecting to %s@%s:%d", sshfs.username, sshfs.host, sshfs.port);
    /* In a real implementation, this would establish SSH connection */
    return 0;
}

static void sshfs_disconnect(void)
{
    sshfs_log("Disconnecting from SSH");
    /* In a real implementation, this would close SSH connection */
}

/* FUSE v3 Operations */

static int sshfs_fuse3_getattr(const char *path, struct stat *stbuf,
                               struct fuse3_file_info *fi)
{
    (void) fi;
    sshfs_log("getattr: %s", path);
    
    memset(stbuf, 0, sizeof(struct stat));
    
    /* Root directory */
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
    }
    
    /* For demo: simulate a few files */
    if (strcmp(path, "/README.txt") == 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = 50;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
    }
    
    return -ENOENT;
}

static int sshfs_fuse3_readdir(const char *path, void *buf, fuse3_fill_dir_t filler,
                              off_t offset, struct fuse3_file_info *fi,
                              enum fuse3_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;
    
    sshfs_log("readdir: %s", path);
    
    if (strcmp(path, "/") != 0)
        return -ENOENT;
    
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, "README.txt", NULL, 0, 0);
    
    return 0;
}

static int sshfs_fuse3_open(const char *path, struct fuse3_file_info *fi)
{
    sshfs_log("open: %s, flags=0x%x", path, fi->flags);
    
    if (strcmp(path, "/README.txt") == 0) {
        if ((fi->flags & O_ACCMODE) == O_RDONLY) {
            return 0;
        }
        return -EACCES;
    }
    
    return -ENOENT;
}

static int sshfs_fuse3_read(const char *path, char *buf, size_t size, off_t offset,
                           struct fuse3_file_info *fi)
{
    (void) fi;
    sshfs_log("read: %s, size=%zu, offset=%lld", path, size, offset);
    
    if (strcmp(path, "/README.txt") == 0) {
        const char *content = "SSHFS with FUSE3 - Connected via eleph-tree layer\n";
        size_t len = strlen(content);
        
        if (offset < (off_t)len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, content + offset, size);
            return size;
        }
        return 0;
    }
    
    return -ENOENT;
}

static int sshfs_fuse3_create(const char *path, mode_t mode,
                             struct fuse3_file_info *fi)
{
    (void) fi;
    sshfs_log("create: %s, mode=0%o", path, mode);
    /* In real implementation, would create file via SFTP */
    return -ENOSYS;
}

static int sshfs_fuse3_write(const char *path, const char *buf, size_t size,
                            off_t offset, struct fuse3_file_info *fi)
{
    (void) path;
    (void) buf;
    (void) size;
    (void) offset;
    (void) fi;
    sshfs_log("write: %s, size=%zu, offset=%lld", path, size, offset);
    /* In real implementation, would write via SFTP */
    return -ENOSYS;
}

static int sshfs_fuse3_mkdir(const char *path, mode_t mode)
{
    sshfs_log("mkdir: %s, mode=0%o", path, mode);
    /* In real implementation, would create directory via SFTP */
    return -ENOSYS;
}

static int sshfs_fuse3_unlink(const char *path)
{
    sshfs_log("unlink: %s", path);
    /* In real implementation, would delete file via SFTP */
    return -ENOSYS;
}

static int sshfs_fuse3_rmdir(const char *path)
{
    sshfs_log("rmdir: %s", path);
    /* In real implementation, would remove directory via SFTP */
    return -ENOSYS;
}

static void *sshfs_fuse3_init(struct fuse3_conn_info *conn,
                             struct fuse3_config *cfg)
{
    (void) conn;
    (void) cfg;
    
    sshfs_log("FUSE3 init");
    
    /* Initialize SSH connection */
    if (sshfs_connect() != 0) {
        fprintf(stderr, "Failed to connect to SSH server\n");
        return NULL;
    }
    
    return &sshfs;
}

static void sshfs_fuse3_destroy(void *private_data)
{
    (void) private_data;
    sshfs_log("FUSE3 destroy");
    sshfs_disconnect();
}

/* FUSE v3 operations structure */
static const struct fuse3_operations sshfs_fuse3_ops = {
    .init       = sshfs_fuse3_init,
    .destroy    = sshfs_fuse3_destroy,
    .getattr    = sshfs_fuse3_getattr,
    .readdir    = sshfs_fuse3_readdir,
    .open       = sshfs_fuse3_open,
    .read       = sshfs_fuse3_read,
    .create     = sshfs_fuse3_create,
    .write      = sshfs_fuse3_write,
    .mkdir      = sshfs_fuse3_mkdir,
    .unlink     = sshfs_fuse3_unlink,
    .rmdir      = sshfs_fuse3_rmdir,
};

/* Command line options */
static void usage(const char *progname)
{
    fprintf(stderr,
            "Usage: %s [options] user@host:[dir] mountpoint\n"
            "\n"
            "SSHFS options:\n"
            "    -p PORT            port to connect to (default: 22)\n"
            "    -o reconnect       reconnect to server on failure\n"
            "    -o follow_symlinks follow symlinks on the server\n"
            "    -o no_check_root   don't check for existence of 'dir' on server\n"
            "    -o debug           enable debug output\n"
            "\n"
            "FUSE options:\n"
            "    -d                 enable debug output (implies -f)\n"
            "    -f                 foreground operation\n"
            "    -s                 disable multi-threaded operation\n"
            "\n", progname);
}

/* Parse connection string: user@host:path */
static int parse_connection(const char *str)
{
    char *s = strdup(str);
    char *p;
    
    /* Extract username */
    p = strchr(s, '@');
    if (!p) {
        fprintf(stderr, "Error: Invalid connection string (missing @)\n");
        free(s);
        return -1;
    }
    *p = '\0';
    sshfs.username = strdup(s);
    s = p + 1;
    
    /* Extract host and optional path */
    p = strchr(s, ':');
    if (p) {
        *p = '\0';
        /* Path after : would be used in real implementation */
    }
    sshfs.host = strdup(s);
    
    free(s - strlen(sshfs.username) - 1);
    return 0;
}

int main(int argc, char *argv[])
{
    struct fuse3_args args = { 0, NULL, 0 };
    int res;
    
    printf("🐘 SSHFS with FUSE3 API (eleph-tree)\n");
    printf("Secure Shell File System using FUSE v3 compatibility layer\n\n");
    
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }
    
    /* Parse connection string (second to last argument) */
    if (parse_connection(argv[argc - 2]) != 0) {
        return 1;
    }
    
    /* Setup FUSE arguments */
    args.argc = argc - 1;  /* Remove connection string */
    args.argv = malloc(sizeof(char*) * args.argc);
    args.argv[0] = argv[0];
    for (int i = 1; i < argc - 2; i++) {
        args.argv[i] = argv[i];
    }
    args.argv[args.argc - 1] = argv[argc - 1];  /* Mount point */
    
    /* Parse options */
    for (int i = 1; i < args.argc - 1; i++) {
        if (strcmp(args.argv[i], "-d") == 0) {
            sshfs.debug = 1;
        } else if (strcmp(args.argv[i], "-o") == 0 && i + 1 < args.argc - 1) {
            if (strcmp(args.argv[i + 1], "debug") == 0) {
                sshfs.debug = 1;
            } else if (strcmp(args.argv[i + 1], "reconnect") == 0) {
                sshfs.reconnect = 1;
            } else if (strcmp(args.argv[i + 1], "follow_symlinks") == 0) {
                sshfs.follow_symlinks = 1;
            } else if (strcmp(args.argv[i + 1], "no_check_root") == 0) {
                sshfs.no_check_root = 1;
            }
        } else if (strcmp(args.argv[i], "-p") == 0 && i + 1 < args.argc - 1) {
            sshfs.port = atoi(args.argv[i + 1]);
        }
    }
    
    printf("Connecting to: %s@%s:%d\n", sshfs.username, sshfs.host, sshfs.port);
    printf("Mount point: %s\n", args.argv[args.argc - 1]);
    
    /* Initialize mutex */
    pthread_mutex_init(&sshfs.lock, NULL);
    
    /* Create FUSE v3 handle */
    struct fuse3 *fuse = fuse3_new(&args, &sshfs_fuse3_ops, 
                                   sizeof(sshfs_fuse3_ops), &sshfs);
    if (!fuse) {
        fprintf(stderr, "Failed to create FUSE v3 handle\n");
        res = 1;
        goto cleanup;
    }
    
    /* Mount filesystem */
    if (fuse3_mount(fuse, args.argv[args.argc - 1]) != 0) {
        fprintf(stderr, "Failed to mount filesystem\n");
        fuse3_destroy(fuse);
        res = 1;
        goto cleanup;
    }
    
    printf("\nSSHFS mounted successfully. Press Ctrl+C to unmount.\n");
    
    /* Run FUSE loop */
    res = fuse3_loop(fuse);
    
    /* Cleanup */
    fuse3_unmount(fuse);
    fuse3_destroy(fuse);
    
cleanup:
    pthread_mutex_destroy(&sshfs.lock);
    free(sshfs.host);
    free(sshfs.username);
    free(args.argv);
    
    return res;
}