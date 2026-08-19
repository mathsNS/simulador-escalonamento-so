#include "scheduler_internal.h"

int scheduler_choose_priority(const RuntimeProcess *processes, size_t count) {
    int best = -1;
    size_t i;
    for (i = 0; i < count; ++i) {
        if (processes[i].state != PROCESS_READY)
            continue;
        if (best < 0 || processes[i].spec.priority < processes[best].spec.priority ||
            (processes[i].spec.priority == processes[best].spec.priority &&
             (processes[i].ready_order < processes[best].ready_order ||
              (processes[i].ready_order == processes[best].ready_order &&
               (processes[i].spec.arrival_time < processes[best].spec.arrival_time ||
                (processes[i].spec.arrival_time == processes[best].spec.arrival_time &&
                 processes[i].spec.id < processes[best].spec.id))))))
            best = (int)i;
    }
    return best;
}
