#include "common.h"
#include "extension.h"
#include "filesystem.h"
#include "memory.h"
#include "scheduler.h"
#include "sync_demo.h"

#include <stdio.h>

static void print_help(void) {
    print_header("帮助");
    printf("本项目包含 4 个基础模块与 1 个扩展模块：\n");
    printf("1. 处理机调度：FCFS / SJF / RR / Priority\n");
    printf("2. 内存管理：FF / BF / FIFO / LRU\n");
    printf("3. 进程同步：生产者消费者 / 读者写者\n");
    printf("4. 文件系统：简化目录与空闲空间管理\n");
    printf("5. 扩展模块：调度算法性能对比\n");
}

int main(void) {
    int choice;

    while (1) {
        print_header("操作系统课程设计");
        printf("1. 处理机调度\n");
        printf("2. 内存管理\n");
        printf("3. 进程同步与并发控制\n");
        printf("4. 文件系统\n");
        printf("5. 调度扩展分析\n");
        printf("6. 帮助\n");
        printf("0. 退出\n");

        choice = prompt_int("请选择功能: ", 0, 6);
        switch (choice) {
            case 1:
                scheduler_menu();
                break;
            case 2:
                memory_menu();
                break;
            case 3:
                sync_menu();
                break;
            case 4:
                filesystem_menu();
                break;
            case 5:
                extension_menu();
                break;
            case 6:
                print_help();
                break;
            case 0:
                print_header("已退出");
                printf("感谢使用。\n");
                return 0;
            default:
                printf("无效选择。\n");
                break;
        }
    }
}
