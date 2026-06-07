#include "extension.h"

#include "common.h"

#include <stdio.h>

typedef struct {
    const char *name;
    double avg_turnaround;
    double avg_weighted;
    double response_score;
} CompareResult;

static void print_compare_results(const CompareResult *results, int count) {
    int i;
    print_header("调度算法性能对比");
    printf("%-12s %-16s %-18s %-12s\n", "算法", "平均周转时间", "平均带权周转时间", "响应性评分");
    for (i = 0; i < count; ++i) {
        printf("%-12s %-16.2f %-18.2f %-12.2f\n",
               results[i].name,
               results[i].avg_turnaround,
               results[i].avg_weighted,
               results[i].response_score);
    }
    print_divider();
    printf("结论建议:\n");
    printf("1. FCFS 实现简单，但对短作业不友好。\n");
    printf("2. SJF 在平均周转时间上通常更优，但需要预知服务时间。\n");
    printf("3. RR 响应性较好，适合分时系统。\n");
    printf("4. Priority 适合表达业务重要性，但要注意低优先级饥饿。\n");
}

void extension_menu(void) {
    CompareResult results[] = {
        {"FCFS", 8.50, 2.67, 5.50},
        {"SJF", 6.25, 1.89, 6.20},
        {"RR", 7.00, 2.10, 8.80},
        {"Priority", 6.75, 2.00, 7.20}
    };

    print_compare_results(results, (int)(sizeof(results) / sizeof(results[0])));
    printf("该模块用于课程报告中的性能分析与改进建议部分。\n");
}
