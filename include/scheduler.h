#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct {
    char name[32];
    int arrival_time;
    int burst_time;
    int priority;
} ProcessInput;

void scheduler_menu(void);

#endif
