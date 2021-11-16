#include <stdio.h>
#define FUSE_USE_VERSION 31

#include "fs.h"
#include <fuse3/fuse.h>
#include <string.h>

struct __data__{
};

struct __data__* get_private_data(){
    return ((struct __data__*)(fuse_get_context()->private_data));
}

static int __create__(const char* path, mode_t mode, struct fuse_file_info* fi){}

static int __getattr__(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {}

static void* __init__(struct fuse_conn_info* conn, struct fuse_config* cfg){
    return NULL;
}

static int __mkdir__(const char* path, mode_t mode){}

static int __mknod__(const char* path, mode_t mode, dev_t dev){}

static int __readdir__(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {} 

static int __read__(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {}

static int __rename__(const char* path, const char* new_name, unsigned int flags){}

static int __rmdir__(const char* path){}

static int __unlink__(const char* path){}

static int __write__(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi){}

static int __write_buf__(const char* path, struct fuse_bufvec* buf, off_t offset, struct fuse_file_info* fi){}

static const struct fuse_operations operations = {
    .create = __create__,
    .getattr = __getattr__,
    .init = __init__,
    .mkdir = __mkdir__,
    .mknod = __mknod__,
    .readdir = __readdir__,
    .read = __read__,
    .rename = __rename__,
    .rmdir = __rmdir__,
    .unlink = __unlink__,
    .write = __write__,
    .write_buf = __write_buf__,
};

void mount_custom_fs(char *mount_point){
    char* args[] = {"NULL", mount_point};
    int ret = 0;
    fuse_main(2, args, &operations, NULL);
}
