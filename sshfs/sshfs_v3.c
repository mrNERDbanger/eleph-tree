/*
 * SSHFS with FUSE v3 API support using eleph-tree compatibility layer
 * This demonstrates how to use FUSE v3 API with SSHFS on macOS
 */

#define FUSE_USE_VERSION 30

#include <fuse3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Simplified SSHFS structure for demo */
struct sshfs_file {
    char *name;
    char *content;
    size_t size;
    mode_t mode;
};

/* Demo files */
static struct sshfs_file demo_files[] = {
    {"hello.txt", "Hello from SSHFS v3!\n", 20, S_IFREG | 0644},
    {"readme.md", "# SSHFS v3 Demo\nThis uses eleph-tree FUSE v3 API!\n", 49, S_IFREG | 0644},
    {NULL, NULL, 0, 0}
};

static int sshfs_v3_getattr(const char *path, struct stat *stbuf, struct fuse3_file_info *fi)
{
    (void) fi;
    
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    /* Check demo files */
    for (int i = 0; demo_files[i].name; i++) {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "/%s", demo_files[i].name);
        
        if (strcmp(path, full_path) == 0) {
            stbuf->st_mode = demo_files[i].mode;
            stbuf->st_nlink = 1;
            stbuf->st_size = demo_files[i].size;
            return 0;
        }
    }
    
    return -ENOENT;
}

static int sshfs_v3_readdir(const char *path, void *buf, fuse3_fill_dir_t filler,
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
    
    /* Add demo files */
    for (int i = 0; demo_files[i].name; i++) {
        filler(buf, demo_files[i].name, NULL, 0, 0);
    }
    
    return 0;
}

static int sshfs_v3_open(const char *path, struct fuse3_file_info *fi)
{
    /* Check if file exists */
    for (int i = 0; demo_files[i].name; i++) {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "/%s", demo_files[i].name);
        
        if (strcmp(path, full_path) == 0) {
            if ((fi->flags & O_ACCMODE) != O_RDONLY)
                return -EACCES;
            return 0;
        }
    }
    
    return -ENOENT;
}

static int sshfs_v3_read(const char *path, char *buf, size_t size, off_t offset,
                        struct fuse3_file_info *fi)
{
    (void) fi;
    
    /* Find the file */
    for (int i = 0; demo_files[i].name; i++) {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "/%s", demo_files[i].name);
        
        if (strcmp(path, full_path) == 0) {
            size_t len = demo_files[i].size;
            if (offset < len) {
                if (offset + size > len)
                    size = len - offset;
                memcpy(buf, demo_files[i].content + offset, size);
            } else {
                size = 0;
            }
            return size;
        }
    }
    
    return -ENOENT;
}

/* FUSE v3 operations structure */
static const struct fuse3_operations sshfs_v3_ops = {
    .getattr    = sshfs_v3_getattr,
    .readdir    = sshfs_v3_readdir,
    .open       = sshfs_v3_open,
    .read       = sshfs_v3_read,
};

int main(int argc, char *argv[])
{
    printf("🐘 SSHFS v3 Demo using eleph-tree FUSE v3 API\n");
    printf("This demonstrates SSHFS with FUSE v3 compatibility on macOS\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <mountpoint>\n", argv[0]);
        printf("Example: %s /tmp/sshfs_mount\n", argv[0]);
        printf("\nAfter mounting, try:\n");
        printf("  ls /tmp/sshfs_mount/\n");
        printf("  cat /tmp/sshfs_mount/hello.txt\n");
        printf("  cat /tmp/sshfs_mount/readme.md\n");
        return 1;
    }
    
    struct fuse3_args args = { argc, argv, 0 };
    
    printf("Creating FUSE v3 filesystem handle...\n");
    struct fuse3 *fuse = fuse3_new(&args, &sshfs_v3_ops, sizeof(sshfs_v3_ops), NULL);
    if (!fuse) {
        fprintf(stderr, "Failed to create FUSE v3 handle\n");
        return 1;
    }
    
    printf("Mounting SSHFS v3 filesystem at: %s\n", argv[1]);
    printf("Files available:\n");
    for (int i = 0; demo_files[i].name; i++) {
        printf("  - %s (%zu bytes)\n", demo_files[i].name, demo_files[i].size);
    }
    printf("\nPress Ctrl+C to unmount\n\n");
    
    if (fuse3_mount(fuse, argv[1]) != 0) {
        fprintf(stderr, "Failed to mount filesystem\n");
        fuse3_destroy(fuse);
        return 1;
    }
    
    int ret = fuse3_loop(fuse);
    
    fuse3_unmount(fuse);
    fuse3_destroy(fuse);
    
    printf("SSHFS v3 filesystem unmounted\n");
    return ret;
}