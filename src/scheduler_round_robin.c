#include "scheduler_internal.h"

/* Round Robin usa a mesma seleção FIFO; a preempção por quantum fica no núcleo. */
int scheduler_choose_round_robin(const PCB *processes, size_t count) {
    return scheduler_choose_fcfs(processes, count);
}
