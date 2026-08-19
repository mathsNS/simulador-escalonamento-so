#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stddef.h>

/*
 * Estados do processo e suas transições:
 *
 *   NEW -> READY                processo chega (arrival_time) e entra na fila
 *                               de prontos.
 *   READY -> RUNNING            escalonador escolhe o processo para usar a CPU.
 *   RUNNING -> READY            preempção (ex.: fim do quantum no Round Robin).
 *   RUNNING -> BLOCKED          processo inicia uma requisição de E/S.
 *   BLOCKED -> READY            requisição de E/S termina; processo volta a
 *                               fila de prontos.
 *   RUNNING -> FINISHED         última rajada de CPU termina.
 *
 * A fila de prontos contém apenas processos em READY; NEW e BLOCKED não
 * disputam a CPU.
 */
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
    SCHED_PRIORITY,
    SCHED_EPA
} SchedulerAlgorithm;

typedef struct {
    int id;
    int arrival_time;
    int priority; /* Um valor menor representa prioridade maior. */
    /*
     * Rajadas alternadas CPU -> E/S -> ... -> CPU. Os índices pares são
     * rajadas de CPU; os ímpares são operações de E/S.
     */
    const int *bursts;
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

/* Retorna 0 em caso de sucesso e -1 se os argumentos/carga forem inválidos. */
int simulator_run(const ProcessSpec *processes, size_t process_count,
                  const SimulationConfig *config, SimulationResult *result);
void simulator_result_destroy(SimulationResult *result);
const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm);

#endif
