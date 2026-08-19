#include "scheduler_internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

int scheduler_choose_round_robin(const RuntimeProcess *, size_t);

static int append_timeline(SimulationResult *result, int value) {
    int *new_items = realloc(result->timeline,
                             (result->timeline_length + 1) * sizeof(*new_items));
    if (!new_items)
        return -1;
    result->timeline = new_items;
    result->timeline[result->timeline_length++] = value;
    return 0;
}

static int validate(const ProcessSpec *specs, size_t count,
                    const SimulationConfig *config) {
    size_t i, j, k;
    if (!specs || !config || count == 0 || config->context_switch_cost < 0 ||
        (config->algorithm == SCHED_ROUND_ROBIN && config->quantum <= 0) ||
        config->algorithm < SCHED_FCFS || config->algorithm > SCHED_PRIORITY)
        return -1;
    for (i = 0; i < count; ++i) {
        if (specs[i].id < 0 || specs[i].arrival_time < 0 || !specs[i].bursts ||
            specs[i].burst_count == 0 || specs[i].burst_count % 2 == 0)
            return -1;
        for (j = 0; j < specs[i].burst_count; ++j)
            if (specs[i].bursts[j] <= 0)
                return -1;
        for (k = i + 1; k < count; ++k)
            if (specs[i].id == specs[k].id)
                return -1;
    }
    return 0;
}

static void make_ready(RuntimeProcess *process, unsigned long long *order) {
    process->state = PROCESS_READY;
    process->ready_order = (*order)++;
}

static void admit_events(RuntimeProcess *processes, size_t count, int time,
                         unsigned long long *order) {
    size_t i;
    /* Chegadas precedem conclusoes de E/S no mesmo instante. */
    for (i = 0; i < count; ++i)
        if (processes[i].state == PROCESS_NEW &&
            processes[i].spec.arrival_time == time)
            make_ready(&processes[i], order);
    for (i = 0; i < count; ++i)
        if (processes[i].state == PROCESS_BLOCKED &&
            processes[i].unblock_time == time)
            make_ready(&processes[i], order);
}

static int record_interval(SimulationResult *result, int enabled, int value,
                           int duration) {
    int i;
    if (!enabled)
        return 0;
    for (i = 0; i < duration; ++i)
        if (append_timeline(result, value) != 0)
            return -1;
    return 0;
}

int simulator_run(const ProcessSpec *specs, size_t count,
                  const SimulationConfig *config, SimulationResult *result) {
    RuntimeProcess *processes = NULL;
    ChooseProcess choose;
    unsigned long long order = 0;
    size_t i, j, finished = 0;
    int time = 0, running = -1, last_pid = SIM_IDLE, quantum_used = 0;

    if (!result)
        return -1;
    memset(result, 0, sizeof(*result));
    if (validate(specs, count, config) != 0)
        return -1;
    processes = calloc(count, sizeof(*processes));
    result->processes = calloc(count, sizeof(*result->processes));
    if (!processes || !result->processes)
        goto fail;
    result->process_count = count;
    for (i = 0; i < count; ++i) {
        processes[i].spec = specs[i];
        processes[i].state = PROCESS_NEW;
        processes[i].remaining = specs[i].bursts[0];
        for (j = 0; j < specs[i].burst_count; j += 2)
            processes[i].total_cpu += specs[i].bursts[j];
        for (j = 1; j < specs[i].burst_count; j += 2)
            processes[i].total_io += specs[i].bursts[j];
    }
    choose = config->algorithm == SCHED_PRIORITY ? scheduler_choose_priority :
             config->algorithm == SCHED_ROUND_ROBIN ? scheduler_choose_round_robin :
             scheduler_choose_fcfs;

    while (finished < count) {
        admit_events(processes, count, time, &order);
        if (running < 0) {
            int next = choose(processes, count);
            if (next < 0) {
                if (record_interval(result, config->record_timeline, SIM_IDLE, 1) != 0)
                    goto fail;
                ++time;
                last_pid = SIM_IDLE;
                continue;
            }
            if (last_pid != SIM_IDLE && last_pid != processes[next].spec.id) {
                int c;
                ++result->context_switches;
                for (c = 0; c < config->context_switch_cost; ++c) {
                    if (record_interval(result, config->record_timeline,
                                        SIM_CONTEXT_SWITCH, 1) != 0)
                        goto fail;
                    ++time;
                    admit_events(processes, count, time, &order);
                }
            }
            running = next;
            processes[running].state = PROCESS_RUNNING;
            quantum_used = 0;
        }

        if (record_interval(result, config->record_timeline,
                            processes[running].spec.id, 1) != 0)
            goto fail;
        --processes[running].remaining;
        ++quantum_used;
        ++time;
        admit_events(processes, count, time, &order);
        last_pid = processes[running].spec.id;

        if (processes[running].remaining == 0) {
            RuntimeProcess *p = &processes[running];
            ++p->burst_index;
            if (p->burst_index == p->spec.burst_count) {
                p->state = PROCESS_FINISHED;
                p->completion_time = time;
                ++finished;
            } else {
                int io_duration = p->spec.bursts[p->burst_index];
                ++p->burst_index;
                p->remaining = p->spec.bursts[p->burst_index];
                p->unblock_time = time + io_duration;
                p->state = PROCESS_BLOCKED;
            }
            running = -1;
        } else if (config->algorithm == SCHED_ROUND_ROBIN &&
                   quantum_used == config->quantum) {
            make_ready(&processes[running], &order);
            running = -1;
        }
    }

    result->finish_time = time;
    for (i = 0; i < count; ++i) {
        result->processes[i].id = processes[i].spec.id;
        result->processes[i].arrival_time = processes[i].spec.arrival_time;
        result->processes[i].completion_time = processes[i].completion_time;
        result->processes[i].total_cpu_time = processes[i].total_cpu;
        result->processes[i].total_io_time = processes[i].total_io;
    }
    free(processes);
    return 0;

fail:
    free(processes);
    simulator_result_destroy(result);
    return -1;
}

void simulator_result_destroy(SimulationResult *result) {
    if (!result)
        return;
    free(result->processes);
    free(result->timeline);
    memset(result, 0, sizeof(*result));
}

const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm) {
    switch (algorithm) {
    case SCHED_FCFS: return "FCFS";
    case SCHED_ROUND_ROBIN: return "Round Robin";
    case SCHED_PRIORITY: return "Prioridade nao preemptiva";
    default: return "Desconhecido";
    }
}
