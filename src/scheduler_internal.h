#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#include "simulator.h"

typedef struct {
    ProcessSpec spec;
    ProcessState state;
    size_t burst_index;
    int remaining;
    int unblock_time;
    int completion_time;
    unsigned long long ready_order;
    int total_cpu;
    int total_io;
} RuntimeProcess;

typedef int (*ChooseProcess)(const RuntimeProcess *, size_t);

int scheduler_choose_fcfs(const RuntimeProcess *processes, size_t count);
int scheduler_choose_priority(const RuntimeProcess *processes, size_t count);

#endif
