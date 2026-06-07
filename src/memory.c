#include "memory.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

#define MAX_BLOCKS 32
#define MAX_PAGES 64

typedef struct {
    int start;
    int size;
    int free;
    char owner[32];
} Partition;

static void init_memory(Partition *blocks, int *count, int total_size) {
    *count = 1;
    blocks[0].start = 0;
    blocks[0].size = total_size;
    blocks[0].free = 1;
    strcpy(blocks[0].owner, "-");
}

static void print_partitions(const Partition *blocks, int count) {
    int i;
    printf("%-8s %-8s %-8s %-8s\n", "起始", "大小", "状态", "进程");
    for (i = 0; i < count; ++i) {
        printf("%-8d %-8d %-8s %-8s\n",
               blocks[i].start,
               blocks[i].size,
               blocks[i].free ? "空闲" : "占用",
               blocks[i].owner);
    }
}

static void merge_free(Partition *blocks, int *count) {
    int i = 0;
    while (i < *count - 1) {
        if (blocks[i].free && blocks[i + 1].free) {
            blocks[i].size += blocks[i + 1].size;
            memmove(&blocks[i + 1], &blocks[i + 2], (size_t)(*count - i - 2) * sizeof(Partition));
            (*count)--;
        } else {
            i++;
        }
    }
}

static int allocate_block(Partition *blocks, int *count, const char *name, int size, int best_fit) {
    int selected = -1;
    int i;
    for (i = 0; i < *count; ++i) {
        if (!blocks[i].free || blocks[i].size < size) {
            continue;
        }
        if (!best_fit) {
            selected = i;
            break;
        }
        if (selected == -1 || blocks[i].size < blocks[selected].size) {
            selected = i;
        }
    }
    if (selected == -1) {
        return 0;
    }

    if (blocks[selected].size == size) {
        blocks[selected].free = 0;
        snprintf(blocks[selected].owner, sizeof(blocks[selected].owner), "%s", name);
        return 1;
    }

    if (*count >= MAX_BLOCKS) {
        return 0;
    }

    memmove(&blocks[selected + 1], &blocks[selected], (size_t)(*count - selected) * sizeof(Partition));
    blocks[selected + 1].start = blocks[selected].start + size;
    blocks[selected + 1].size = blocks[selected].size - size;
    blocks[selected + 1].free = 1;
    strcpy(blocks[selected + 1].owner, "-");

    blocks[selected].size = size;
    blocks[selected].free = 0;
    snprintf(blocks[selected].owner, sizeof(blocks[selected].owner), "%s", name);
    (*count)++;
    return 1;
}

static int release_block(Partition *blocks, int *count, const char *name) {
    int i;
    for (i = 0; i < *count; ++i) {
        if (!blocks[i].free && strcmp(blocks[i].owner, name) == 0) {
            blocks[i].free = 1;
            strcpy(blocks[i].owner, "-");
            merge_free(blocks, count);
            return 1;
        }
    }
    return 0;
}

static void partition_demo(int best_fit) {
    Partition blocks[MAX_BLOCKS];
    int count;
    int total_size = prompt_int("请输入总内存大小: ", 64, 4096);
    int operations = prompt_int("请输入操作次数: ", 1, 20);
    int i;

    init_memory(blocks, &count, total_size);
    print_header(best_fit ? "动态分区分配 - BF" : "动态分区分配 - FF");
    for (i = 0; i < operations; ++i) {
        char name[32];
        int action;
        print_divider();
        print_partitions(blocks, count);
        action = prompt_int("1=分配 2=回收: ", 1, 2);
        prompt_string("进程名: ", name, sizeof(name));
        if (action == 1) {
            int size = prompt_int("请求大小: ", 1, total_size);
            if (allocate_block(blocks, &count, name, size, best_fit)) {
                printf("分配成功。\n");
            } else {
                printf("分配失败，可能是空间不足或碎片过多。\n");
            }
        } else {
            if (release_block(blocks, &count, name)) {
                printf("回收成功。\n");
            } else {
                printf("未找到对应进程。\n");
            }
        }
    }
    print_divider();
    print_partitions(blocks, count);
}

static void page_replacement_demo(int use_lru) {
    int pages[MAX_PAGES];
    int frames[16];
    int last_used[16] = {0};
    int frame_count = prompt_int("请输入页框数量(1-16): ", 1, 16);
    int ref_count = prompt_int("请输入访问页数(1-64): ", 1, MAX_PAGES);
    int page_faults = 0;
    int pointer = 0;
    int clock = 0;
    int i;

    for (i = 0; i < frame_count; ++i) {
        frames[i] = -1;
    }
    for (i = 0; i < ref_count; ++i) {
        char label[64];
        snprintf(label, sizeof(label), "第 %d 次访问页号: ", i + 1);
        pages[i] = prompt_int(label, 0, 1000);
    }

    print_header(use_lru ? "页面置换 - LRU" : "页面置换 - FIFO");
    for (i = 0; i < ref_count; ++i) {
        int hit = 0;
        int j;
        clock++;
        for (j = 0; j < frame_count; ++j) {
            if (frames[j] == pages[i]) {
                hit = 1;
                last_used[j] = clock;
                break;
            }
        }
        if (!hit) {
            int replace = -1;
            page_faults++;
            for (j = 0; j < frame_count; ++j) {
                if (frames[j] == -1) {
                    replace = j;
                    break;
                }
            }
            if (replace == -1) {
                if (use_lru) {
                    replace = 0;
                    for (j = 1; j < frame_count; ++j) {
                        if (last_used[j] < last_used[replace]) {
                            replace = j;
                        }
                    }
                } else {
                    replace = pointer;
                    pointer = (pointer + 1) % frame_count;
                }
            }
            frames[replace] = pages[i];
            last_used[replace] = clock;
        }

        printf("访问页 %d => %s | 页框: ", pages[i], hit ? "命中" : "缺页");
        for (j = 0; j < frame_count; ++j) {
            if (frames[j] == -1) {
                printf("[ ] ");
            } else {
                printf("[%d] ", frames[j]);
            }
        }
        printf("\n");
    }

    printf("缺页次数: %d\n", page_faults);
    printf("缺页率: %.2f%%\n", 100.0 * page_faults / ref_count);
}

void memory_menu(void) {
    int choice;

    while (1) {
        print_header("内存管理");
        printf("1. 首次适应 FF\n");
        printf("2. 最佳适应 BF\n");
        printf("3. FIFO 页面置换\n");
        printf("4. LRU 页面置换\n");
        printf("0. 返回上级菜单\n");
        choice = prompt_int("请选择功能: ", 0, 4);
        switch (choice) {
            case 1:
                partition_demo(0);
                break;
            case 2:
                partition_demo(1);
                break;
            case 3:
                page_replacement_demo(0);
                break;
            case 4:
                page_replacement_demo(1);
                break;
            case 0:
                return;
            default:
                printf("无效选择。\n");
                break;
        }
    }
}
