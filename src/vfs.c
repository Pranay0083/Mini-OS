#include "../include/vfs.h"
#include "../include/string.h"
#include "../include/memory.h"
#include "../include/screen.h"

static File files[MAX_FILES];

void vfs_init(void)
{
  for (int i = 0; i < MAX_FILES; i++) {
    files[i].used = 0;
  }
}

static int find_file(const char *name)
{
    if (!name) return -1;
    for (int i = 0; i < MAX_FILES; i++) {
      if (files[i].used && str_compare(files[i].name, name) == 0)
          return i;
    }
    return -1;
}

int vfs_create(const char *name)
{
    if (!name || str_length(name) == 0) {
      scr_println("Error: filename required");
      return -1;
    }

    if (find_file(name) != -1) {
        return 0;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            files[i].used = 1;
            files[i].size = 0;
            files[i].data = 0;
            str_copy(files[i].name, name, MAX_NAME_LEN);
            return 0;
        }
    }

    scr_println("File table full");
    return -1;
}

int vfs_write(const char *name, const char *data)
{
    if (!name || !data) {
        scr_println("Error: invalid arguments");
        return -1;
    }

    int idx = find_file(name);

    /* Auto-create file if it doesn't exist */
    if (idx == -1) {
        if (vfs_create(name) != 0) return -1;
        idx = find_file(name);
        if (idx == -1) return -1;
    }

    int len = str_length(data);

    char *mem = (char *)mem_alloc(len + 1);
    if (!mem) {
        scr_println("Error: out of memory");
        return -1;
    }

    str_copy(mem, data, len + 1);

    /* free old data if exists */
    if (files[idx].data) {
        mem_free(files[idx].data);
    }

    files[idx].data = mem;
    files[idx].size = len;

    /* Confirmation */
    char buf[16];
    str_itoa(len, buf, 16);
    scr_print("Written ");
    scr_print(buf);
    scr_print(" bytes to '");
    scr_print(name);
    scr_println("'");

    return 0;
}

char *vfs_read(const char *name)
{
    if (!name) {
        scr_println("Error: filename required");
        return 0;
    }

    int idx = find_file(name);
    if (idx == -1) {
        scr_println("File not found");
        return 0;
    }

    return files[idx].data;
}

int vfs_delete(const char *name)
{
    if (!name) {
        scr_println("Error: filename required");
        return -1;
    }

    int idx = find_file(name);
    if (idx == -1) {
        scr_println("File not found");
        return -1;
    }

    if (files[idx].data) {
        mem_free(files[idx].data);
    }

    files[idx].used = 0;

    scr_print("Removed '");
    scr_print(name);
    scr_println("'");
    return 0;
}

void vfs_list(int show_hidden)
{
    int count = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used) {
            if (!show_hidden && files[i].name[0] == '.') continue;
            
            scr_print("  ");
            scr_print(files[i].name);

            char buf[16];
            str_itoa(files[i].size, buf, 16);
            scr_print("  ");
            scr_print(buf);
            scr_println(" B");

            count++;
        }
    }

    if (count == 0) {
        scr_println("(empty)");
    }

    char cbuf[16];
    str_itoa(count, cbuf, 16);
    scr_print(cbuf);
    scr_println(" file(s)");
}