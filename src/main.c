#include "simulator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_algorithm(const char *name, SchedulerAlgorithm *algorithm) {
    if (strcmp(name, "fcfs") == 0) *algorithm = SCHED_FCFS;
    else if (strcmp(name, "rr") == 0) *algorithm = SCHED_ROUND_ROBIN;
    else if (strcmp(name, "priority") == 0) *algorithm = SCHED_PRIORITY;
    else return -1;
    return 0;
}

int main(int argc, char **argv) {
    static const int p1_bursts[] = {3, 2, 2};
    static const int p2_bursts[] = {4};
    static const int p3_bursts[] = {2};
    const ProcessSpec workload[] = {
        {1, 0, 2, p1_bursts, 3},
        {2, 1, 1, p2_bursts, 1},
        {3, 3, 3, p3_bursts, 1}
    };
    SimulationConfig config = {SCHED_FCFS, 2, 1, 1};
    SimulationResult result;
    size_t i;

    if (argc > 1 && parse_algorithm(argv[1], &config.algorithm) != 0) {
        fprintf(stderr, "Uso: %s [fcfs|rr|priority] [quantum] [custo-troca]\n", argv[0]);
        return 2;
    }
    if (argc > 2) config.quantum = atoi(argv[2]);
    if (argc > 3) config.context_switch_cost = atoi(argv[3]);
    if (simulator_run(workload, 3, &config, &result) != 0) {
        fprintf(stderr, "Configuracao invalida ou falha na simulacao.\n");
        return 1;
    }
    printf("Algoritmo: %s\nTempo final: %d\nTrocas de contexto: %d\nLinha do tempo:",
           scheduler_algorithm_name(config.algorithm), result.finish_time,
           result.context_switches);
    for (i = 0; i < result.timeline_length; ++i) {
        if (result.timeline[i] == SIM_IDLE) printf(" idle");
        else if (result.timeline[i] == SIM_CONTEXT_SWITCH) printf(" cs");
        else printf(" P%d", result.timeline[i]);
    }
    putchar('\n');
    for (i = 0; i < result.process_count; ++i)
        printf("P%d: conclusao=%d turnaround=%d CPU=%d E/S=%d\n",
               result.processes[i].id, result.processes[i].completion_time,
               result.processes[i].completion_time - result.processes[i].arrival_time,
               result.processes[i].total_cpu_time, result.processes[i].total_io_time);
    simulator_result_destroy(&result);
    return 0;
}
