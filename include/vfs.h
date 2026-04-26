#ifndef VFS_H
#define VFS_H

#define MAX_FILES 32
#define MAX_NAME_LEN 32

typedef struct File {
    char name[MAX_NAME_LEN];
    char *data;
    int size;
    int used;
} File;

void vfs_init(void);
int  vfs_create(const char *name);
int  vfs_write(const char *name, const char *data);
char *vfs_read(const char *name);
int  vfs_delete(const char *name);
void vfs_list(int show_hidden);

#endif