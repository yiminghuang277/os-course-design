#include "filesystem.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

#define MAX_FILES 32
#define DISK_SIZE 1024

typedef struct {
    char name[32];
    char content[256];
    int size;
    int start;
    int in_use;
} VirtualFile;

typedef struct {
    VirtualFile files[MAX_FILES];
    int used_space;
} FileSystemState;

static void compact_files(FileSystemState *fs) {
    int cursor = 0;
    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        if (fs->files[i].in_use) {
            fs->files[i].start = cursor;
            cursor += fs->files[i].size;
        }
    }
    fs->used_space = cursor;
}

static void init_fs(FileSystemState *fs) {
    memset(fs, 0, sizeof(*fs));
}

static int find_file(const FileSystemState *fs, const char *name) {
    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        if (fs->files[i].in_use && strcmp(fs->files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int first_empty_slot(const FileSystemState *fs) {
    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        if (!fs->files[i].in_use) {
            return i;
        }
    }
    return -1;
}

static int create_file(FileSystemState *fs, const char *name) {
    int slot;
    if (find_file(fs, name) != -1) {
        return 0;
    }
    slot = first_empty_slot(fs);
    if (slot == -1) {
        return 0;
    }
    fs->files[slot].in_use = 1;
    fs->files[slot].size = 0;
    fs->files[slot].start = fs->used_space;
    fs->files[slot].content[0] = '\0';
    snprintf(fs->files[slot].name, sizeof(fs->files[slot].name), "%s", name);
    return 1;
}

static int write_file(FileSystemState *fs, const char *name, const char *content) {
    int index = find_file(fs, name);
    int new_size;
    if (index == -1) {
        return -1;
    }
    new_size = (int)strlen(content);
    if (fs->used_space - fs->files[index].size + new_size > DISK_SIZE) {
        return 0;
    }
    fs->used_space = fs->used_space - fs->files[index].size + new_size;
    fs->files[index].size = new_size;
    snprintf(fs->files[index].content, sizeof(fs->files[index].content), "%s", content);
    compact_files(fs);
    return 1;
}

static int delete_file(FileSystemState *fs, const char *name) {
    int index = find_file(fs, name);
    if (index == -1) {
        return 0;
    }
    memset(&fs->files[index], 0, sizeof(fs->files[index]));
    compact_files(fs);
    return 1;
}

static void read_file_content(const FileSystemState *fs, const char *name) {
    int index = find_file(fs, name);
    if (index == -1) {
        printf("文件不存在。\n");
        return;
    }
    printf("文件名: %s\n", fs->files[index].name);
    printf("大小: %d\n", fs->files[index].size);
    printf("内容: %s\n", fs->files[index].content);
}

static void list_files(const FileSystemState *fs) {
    int i;
    print_header("目录列表");
    printf("总容量: %d, 已使用: %d, 空闲: %d\n", DISK_SIZE, fs->used_space, DISK_SIZE - fs->used_space);
    printf("%-12s %-8s %-8s\n", "文件名", "大小", "起始块");
    for (i = 0; i < MAX_FILES; ++i) {
        if (fs->files[i].in_use) {
            printf("%-12s %-8d %-8d\n", fs->files[i].name, fs->files[i].size, fs->files[i].start);
        }
    }
}

void filesystem_menu(void) {
    static FileSystemState fs;
    static int initialized = 0;
    int choice;
    char name[32];
    char content[256];

    if (!initialized) {
        init_fs(&fs);
        initialized = 1;
    }

    while (1) {
        print_header("简化文件系统");
        printf("1. 创建文件\n");
        printf("2. 写入文件\n");
        printf("3. 读取文件\n");
        printf("4. 删除文件\n");
        printf("5. 列出目录\n");
        printf("0. 返回上级菜单\n");
        choice = prompt_int("请选择功能: ", 0, 5);
        switch (choice) {
            case 1:
                prompt_string("请输入文件名: ", name, sizeof(name));
                if (create_file(&fs, name)) {
                    printf("创建成功。\n");
                } else {
                    printf("创建失败，可能文件已存在或目录已满。\n");
                }
                break;
            case 2:
                prompt_string("请输入文件名: ", name, sizeof(name));
                prompt_string("请输入内容: ", content, sizeof(content));
                switch (write_file(&fs, name, content)) {
                    case 1:
                        printf("写入成功。\n");
                        break;
                    case 0:
                        printf("写入失败，磁盘空间不足。\n");
                        break;
                    default:
                        printf("文件不存在。\n");
                        break;
                }
                break;
            case 3:
                prompt_string("请输入文件名: ", name, sizeof(name));
                read_file_content(&fs, name);
                break;
            case 4:
                prompt_string("请输入文件名: ", name, sizeof(name));
                if (delete_file(&fs, name)) {
                    printf("删除成功。\n");
                } else {
                    printf("文件不存在。\n");
                }
                break;
            case 5:
                list_files(&fs);
                break;
            case 0:
                return;
            default:
                printf("无效选择。\n");
                break;
        }
    }
}
