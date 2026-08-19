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
    PROCESS_NEW,       /* Ainda não chegou (tempo atual < arrival_time). */
    PROCESS_READY,     /* Apto a usar a CPU, aguardando na fila de prontos. */
    PROCESS_RUNNING,   /* Em posse da CPU. */
    PROCESS_BLOCKED,   /* Aguardando conclusão de uma requisição de E/S. */
    PROCESS_FINISHED   /* Todas as rajadas (CPU e E/S) já concluídas. */
} ProcessState;

typedef enum {
    SCHED_FCFS,
    SCHED_ROUND_ROBIN,
    SCHED_PRIORITY
} SchedulerAlgorithm;

/*
 * Descrição estática de um processo, gerada pela carga de trabalho.
 * Junto com o estado dinâmico, forma o modelo de 
 * processo: id, tempo de chegada, prioridade, 
 * rajadas de CPU/E-S e estado atual.
 */
typedef struct {
    int id; /* Identificador do processo. */
    int arrival_time; /* Tempo de chegada. */
    int priority; /* Um valor menor representa prioridade maior. */
    /*
     * Rajadas alternadas CPU -> E/S -> CPU -> ... -> CPU (indice 0 e sempre
     * CPU e o vetor sempre termina em CPU, então burst_count é ímpar).
     * Índices pares (0, 2, 4, ...) são durações de rajada de CPU; índices
     * impares (1, 3, 5, ...) são durações de requisição de E/S.
     *
     * O número de requisições de E/S do processo é burst_count / 2.
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

/* Retorna 0 em caso de sucesso e -1 se os argumentos/carga forem invalidos. */
int simulator_run(const ProcessSpec *processes, size_t process_count,
                  const SimulationConfig *config, SimulationResult *result);
void simulator_result_destroy(SimulationResult *result);
const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm);

#endif
