/*
 * Hello World example using FUSE v3 API
 * This demonstrates the eleph-tree FUSE v3 compatibility layer
 */

#define FUSE_USE_VERSION 30

#include "fuse3.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

static const char *hello_str = "Hello World from FUSE v3!\n";
static const char *hello_path = "/hello";

static int hello_getattr(const char *path, struct stat *stbuf, struct fuse3_file_info *fi)
{
    (void) fi;
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else
        res = -ENOENT;

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse3_fill_dir_t filler,
                         off_t offset, struct fuse3_file_info *fi,
                         enum fuse3_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    filler(buf, hello_path + 1, NULL, 0, 0);

    return 0;
}

static int hello_open(const char *path, struct fuse3_file_info *fi)
{
    if (strcmp(path, hello_path) != 0)
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse3_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, hello_path) != 0)
        return -ENOENT;

    len = strlen(hello_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, hello_str + offset, size);
    } else
        size = 0;

    return size;
}

static const struct fuse3_operations hello_oper = {
    .getattr    = hello_getattr,
    .readdir    = hello_readdir,
    .open       = hello_open,
    .read       = hello_read,
};

int main(int argc, char *argv[])
{
    printf("🐘 eleph-tree Hello World FUSE v3 Example\n");
    printf("This demonstrates FUSE v3 API compatibility on macOS\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <mountpoint>\n", argv[0]);
        printf("Example: %s /tmp/hello_mount\n", argv[0]);
        return 1;
    }
    
    struct fuse3_args args = { argc, argv, 0 };
    
    struct fuse3 *fuse = fuse3_new(&args, &hello_oper, sizeof(hello_oper), NULL);
    if (!fuse) {
        fprintf(stderr, "Failed to create FUSE v3 handle\n");
        return 1;
    }
    
    printf("Mounting FUSE v3 filesystem at: %s\n", argv[1]);
    printf("Try: cat %s/hello\n", argv[1]);
    printf("Press Ctrl+C to unmount\n\n");
    
    if (fuse3_mount(fuse, argv[1]) != 0) {
        fprintf(stderr, "Failed to mount filesystem\n");
        fuse3_destroy(fuse);
        return 1;
    }
    
    int ret = fuse3_loop(fuse);
    
    fuse3_unmount(fuse);
    fuse3_destroy(fuse);
    
    printf("Filesystem unmounted\n");
    return ret;
}