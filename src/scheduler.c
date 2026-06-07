#include "scheduler.h"

#include "common.h"

#include <stdio.h>
#include <string.h>

#define MAX_PROCESSES 16
#define MAX_TIMELINE 256

typedef struct {
    ProcessInput input;
    int remaining_time;
    int start_time;
    int finish_time;
    int turnaround_time;
    double weighted_turnaround;
    int started;
} ProcessState;

typedef struct {
    char labels[MAX_TIMELINE][32];
    int count;
} Timeline;

static void reset_states(const ProcessInput *inputs, ProcessState *states, int count) {
    int i;
    for (i = 0; i < count; ++i) {
        states[i].input = inputs[i];
        states[i].remaining_time = inputs[i].burst_time;
        states[i].start_time = -1;
        states[i].finish_time = 0;
        states[i].turnaround_time = 0;
        states[i].weighted_turnaround = 0.0;
        states[i].started = 0;
    }
}

static void push_timeline(Timeline *timeline, const char *name) {
    if (timeline->count < MAX_TIMELINE) {
        snprintf(timeline->labels[timeline->count], sizeof(timeline->labels[timeline->count]), "%s", name);
        timeline->count++;
    }
}

static int all_finished(ProcessState *states, int count) {
    int i;
    for (i = 0; i < count; ++i) {
        if (states[i].remaining_time > 0) {
            return 0;
        }
    }
    return 1;
}

static void finalize_metrics(ProcessState *states, int count) {
    int i;
    for (i = 0; i < count; ++i) {
        states[i].turnaround_time = states[i].finish_time - states[i].input.arrival_time;
        states[i].weighted_turnaround = (double)states[i].turnaround_time / states[i].input.burst_time;
    }
}

static void print_results(const char *algorithm, ProcessState *states, int count, const Timeline *timeline) {
    int i;
    double avg_turnaround = 0.0;
    double avg_weighted = 0.0;

    print_header(algorithm);
    printf("执行顺序: ");
    for (i = 0; i < timeline->count; ++i) {
        printf("%s%s", timeline->labels[i], i == timeline->count - 1 ? "\n" : " -> ");
    }
    if (timeline->count == 0) {
        printf("(空)\n");
    }
    print_divider();
    printf("%-8s %-8s %-8s %-8s %-8s %-10s\n", "进程", "到达", "服务", "开始", "完成", "周转/带权");
    for (i = 0; i < count; ++i) {
        avg_turnaround += states[i].turnaround_time;
        avg_weighted += states[i].weighted_turnaround;
        printf("%-8s %-8d %-8d %-8d %-8d %d / %.2f\n",
               states[i].input.name,
               states[i].input.arrival_time,
               states[i].input.burst_time,
               states[i].start_time,
               states[i].finish_time,
               states[i].turnaround_time,
               states[i].weighted_turnaround);
    }
    printf("平均周转时间: %.2f\n", avg_turnaround / count);
    printf("平均带权周转时间: %.2f\n", avg_weighted / count);
}

static void run_fcfs(const ProcessInput *inputs, int count) {
    ProcessState states[MAX_PROCESSES];
    Timeline timeline = {0};
    int finished = 0;
    int current_time = 0;

    reset_states(inputs, states, count);
    while (finished < count) {
        int chosen = -1;
        int i;
        for (i = 0; i < count; ++i) {
            if (states[i].remaining_time <= 0 || states[i].input.arrival_time > current_time) {
                continue;
            }
            if (chosen == -1 ||
                states[i].input.arrival_time < states[chosen].input.arrival_time ||
                (states[i].input.arrival_time == states[chosen].input.arrival_time && i < chosen)) {
                chosen = i;
            }
        }
        if (chosen == -1) {
            current_time++;
            continue;
        }
        states[chosen].start_time = current_time;
        push_timeline(&timeline, states[chosen].input.name);
        current_time += states[chosen].remaining_time;
        states[chosen].remaining_time = 0;
        states[chosen].finish_time = current_time;
        finished++;
    }
    finalize_metrics(states, count);
    print_results("FCFS 调度结果", states, count, &timeline);
}

static void run_sjf(const ProcessInput *inputs, int count) {
    ProcessState states[MAX_PROCESSES];
    Timeline timeline = {0};
    int finished = 0;
    int current_time = 0;

    reset_states(inputs, states, count);
    while (finished < count) {
        int chosen = -1;
        int i;
        for (i = 0; i < count; ++i) {
            if (states[i].remaining_time <= 0 || states[i].input.arrival_time > current_time) {
                continue;
            }
            if (chosen == -1 ||
                states[i].input.burst_time < states[chosen].input.burst_time ||
                (states[i].input.burst_time == states[chosen].input.burst_time &&
                 states[i].input.arrival_time < states[chosen].input.arrival_time)) {
                chosen = i;
            }
        }
        if (chosen == -1) {
            current_time++;
            continue;
        }
        states[chosen].start_time = current_time;
        push_timeline(&timeline, states[chosen].input.name);
        current_time += states[chosen].remaining_time;
        states[chosen].remaining_time = 0;
        states[chosen].finish_time = current_time;
        finished++;
    }
    finalize_metrics(states, count);
    print_results("SJF 调度结果", states, count, &timeline);
}

static void run_priority(const ProcessInput *inputs, int count) {
    ProcessState states[MAX_PROCESSES];
    Timeline timeline = {0};
    int finished = 0;
    int current_time = 0;

    reset_states(inputs, states, count);
    while (finished < count) {
        int chosen = -1;
        int i;
        for (i = 0; i < count; ++i) {
            if (states[i].remaining_time <= 0 || states[i].input.arrival_time > current_time) {
                continue;
            }
            if (chosen == -1 ||
                states[i].input.priority < states[chosen].input.priority ||
                (states[i].input.priority == states[chosen].input.priority &&
                 states[i].input.arrival_time < states[chosen].input.arrival_time)) {
                chosen = i;
            }
        }
        if (chosen == -1) {
            current_time++;
            continue;
        }
        states[chosen].start_time = current_time;
        push_timeline(&timeline, states[chosen].input.name);
        current_time += states[chosen].remaining_time;
        states[chosen].remaining_time = 0;
        states[chosen].finish_time = current_time;
        finished++;
    }
    finalize_metrics(states, count);
    print_results("优先级调度结果", states, count, &timeline);
}

static void run_rr(const ProcessInput *inputs, int count, int quantum) {
    ProcessState states[MAX_PROCESSES];
    Timeline timeline = {0};
    int queue[MAX_TIMELINE];
    int in_queue[MAX_PROCESSES] = {0};
    int front = 0;
    int rear = 0;
    int current_time = 0;
    int i;

    reset_states(inputs, states, count);
    while (!all_finished(states, count)) {
        int progressed = 0;
        for (i = 0; i < count; ++i) {
            if (states[i].remaining_time > 0 &&
                states[i].input.arrival_time <= current_time &&
                !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        if (front == rear) {
            current_time++;
            continue;
        }

        {
            int index = queue[front++];
            int slice = states[index].remaining_time < quantum ? states[index].remaining_time : quantum;
            in_queue[index] = 0;
            if (!states[index].started) {
                states[index].started = 1;
                states[index].start_time = current_time;
            }
            push_timeline(&timeline, states[index].input.name);
            current_time += slice;
            states[index].remaining_time -= slice;
            progressed = 1;

            for (i = 0; i < count; ++i) {
                if (states[i].remaining_time > 0 &&
                    states[i].input.arrival_time <= current_time &&
                    !in_queue[i] &&
                    i != index) {
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            if (states[index].remaining_time > 0) {
                queue[rear++] = index;
                in_queue[index] = 1;
            } else {
                states[index].finish_time = current_time;
            }
        }
        if (!progressed) {
            current_time++;
        }
    }

    finalize_metrics(states, count);
    print_results("RR 调度结果", states, count, &timeline);
}

static void load_sample(ProcessInput *inputs, int *count, int *quantum) {
    *count = 4;
    strcpy(inputs[0].name, "P1");
    inputs[0].arrival_time = 0;
    inputs[0].burst_time = 5;
    inputs[0].priority = 3;
    strcpy(inputs[1].name, "P2");
    inputs[1].arrival_time = 1;
    inputs[1].burst_time = 3;
    inputs[1].priority = 1;
    strcpy(inputs[2].name, "P3");
    inputs[2].arrival_time = 2;
    inputs[2].burst_time = 8;
    inputs[2].priority = 4;
    strcpy(inputs[3].name, "P4");
    inputs[3].arrival_time = 3;
    inputs[3].burst_time = 2;
    inputs[3].priority = 2;
    *quantum = 2;
}

static void input_processes(ProcessInput *inputs, int *count, int *quantum) {
    int i;
    *count = prompt_int("请输入进程数量(1-16): ", 1, MAX_PROCESSES);
    for (i = 0; i < *count; ++i) {
        printf("\n录入进程 P%d\n", i + 1);
        prompt_string("名称: ", inputs[i].name, sizeof(inputs[i].name));
        inputs[i].arrival_time = prompt_int("到达时间: ", 0, 1000);
        inputs[i].burst_time = prompt_int("服务时间: ", 1, 1000);
        inputs[i].priority = prompt_int("优先级(数值越小越高): ", 1, 100);
    }
    *quantum = prompt_int("时间片大小: ", 1, 100);
}

void scheduler_menu(void) {
    ProcessInput inputs[MAX_PROCESSES];
    int count = 0;
    int quantum = 2;
    int choice;

    print_header("处理机调度");
    if (prompt_yes_no("是否加载内置示例数据")) {
        load_sample(inputs, &count, &quantum);
    } else {
        input_processes(inputs, &count, &quantum);
    }

    while (1) {
        print_divider();
        printf("1. FCFS\n");
        printf("2. SJF\n");
        printf("3. RR\n");
        printf("4. Priority\n");
        printf("5. 全部运行\n");
        printf("0. 返回上级菜单\n");
        choice = prompt_int("请选择算法: ", 0, 5);
        switch (choice) {
            case 1:
                run_fcfs(inputs, count);
                break;
            case 2:
                run_sjf(inputs, count);
                break;
            case 3:
                run_rr(inputs, count, quantum);
                break;
            case 4:
                run_priority(inputs, count);
                break;
            case 5:
                run_fcfs(inputs, count);
                run_sjf(inputs, count);
                run_rr(inputs, count, quantum);
                run_priority(inputs, count);
                break;
            case 0:
                return;
            default:
                printf("无效选择。\n");
                break;
        }
    }
}
