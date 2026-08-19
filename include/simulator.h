#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stddef.h>

typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_FINISHED
} ProcessState;

typedef enum {
    SCHED_FCFS,
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY
} SchedulerAlgorithm;

typedef struct {
    int id;
    int arrival_time;
    int priority; /* Um valor menor representa prioridade maior. */
    const int *bursts; /* CPU, E/S, CPU, ...; sempre comeca e termina em CPU. */
    size_t burst_count;
} ProcessSpec;

typedef struct {
    SchedulerAlgorithm algorithm;
    int quantum;
    int context_switch_cost;
    int record_timeline;
} SimulationConfig;

typedef struct {
    int id;
    int arrival_time;
    int completion_time;
    int total_cpu_time;
    int total_io_time;
} ProcessResult;

typedef struct {
    int finish_time;
    int context_switches;
    ProcessResult *processes;
    size_t process_count;
    int *timeline; /* PID, SIM_IDLE ou SIM_CONTEXT_SWITCH em cada unidade. */
    size_t timeline_length;
} SimulationResult;

#define SIM_IDLE (-1)
#define SIM_CONTEXT_SWITCH (-2)

/* Retorna 0 em caso de sucesso e -1 se os argumentos/carga forem invalidos. */
int simulator_run(const ProcessSpec *processes, size_t process_count,
                  const SimulationConfig *config, SimulationResult *result);
void simulator_result_destroy(SimulationResult *result);
const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm);

#endif
