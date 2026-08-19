#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "workload.h"

/*
 * Os 4 cenários obrigatórios da seção 6 da especificação. Cada um é um
 * preset fixo de WorkloadParams; o único grau de liberdade por execução é 
 * process_count e a seed, passados separadamente para scenario_params() e 
 * depois para workload_generate().
 */
typedef enum {
    SCENARIO_BALANCED,             /* Aleatório equilibrado. */
    SCENARIO_IO_BOUND,             /* I/O-bound. */
    SCENARIO_CPU_BOUND,            /* CPU-bound / processos longos. */
    SCENARIO_UNBALANCED_PRIORITY   /* Prioridades desbalanceadas. */
} ScenarioKind;

/* Nome legível do cenário, para relatórios e saída de CLI. */
const char *scenario_name(ScenarioKind kind);

/*
 * Preset de WorkloadParams do cenário para `process_count` processos. Os
 * mesmos parâmetros (exceto a seed) devem ser usados por todos os
 * algoritmos avaliados dentro do cenário.
 */
WorkloadParams scenario_params(ScenarioKind kind, size_t process_count);

#endif
