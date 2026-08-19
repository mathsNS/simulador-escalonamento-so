#ifndef SCENARIOS_H
#define SCENARIOS_H

#include "workload.h"

/* Presets dos quatro cenários avaliados nos experimentos. */
typedef enum {
    SCENARIO_BALANCED,             /* Aleatório equilibrado. */
    SCENARIO_IO_BOUND,             /* I/O-bound. */
    SCENARIO_CPU_BOUND,            /* CPU-bound / processos longos. */
    SCENARIO_UNBALANCED_PRIORITY   /* Prioridades desbalanceadas. */
} ScenarioKind;

/* Nome legível do cenário. */
const char *scenario_name(ScenarioKind kind);

/* Retorna os parâmetros do cenário para `process_count` processos. */
WorkloadParams scenario_params(ScenarioKind kind, size_t process_count);

#endif
