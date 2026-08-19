#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#include "simulator.h"

/* Estado de execução de um processo durante a simulação. */
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

    /* Histórico observado pelo EPA; os escalonadores clássicos não o usam. */
    double epa_predicted_burst;
    long long epa_waiting_ticks;
    int epa_observed_cpu_time;
    int epa_observed_io_time;
} PCB;

/* Estimativa usada pelo EPA antes da primeira rajada observada. */
#define EPA_INITIAL_BURST_ESTIMATE 5.0

typedef int (*ChooseProcess)(const PCB *, size_t);

int scheduler_choose_fcfs(const PCB *processes, size_t count);
int scheduler_choose_priority(const PCB *processes, size_t count);
int scheduler_choose_epa(const PCB *processes, size_t count);

#endif
