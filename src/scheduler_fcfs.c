#include "scheduler_internal.h"

int scheduler_choose_fcfs(const RuntimeProcess *processes, size_t count) {
    int best = -1;
    size_t i;
    for (i = 0; i < count; ++i) {
        if (processes[i].state != PROCESS_READY)
            continue;
        if (best < 0 || processes[i].ready_order < processes[best].ready_order ||
            (processes[i].ready_order == processes[best].ready_order &&
             processes[i].spec.id < processes[best].spec.id))
            best = (int)i;
    }
    return best;
}
