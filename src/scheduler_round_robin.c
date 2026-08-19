#include "scheduler_internal.h"

/* Round Robin usa a mesma selecao FIFO; a preempcao por quantum fica no nucleo. */
int scheduler_choose_round_robin(const RuntimeProcess *processes, size_t count) {
    return scheduler_choose_fcfs(processes, count);
}
