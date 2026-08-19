#include "scenarios.h"
#include "simulator.h"
#include "workload.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Executor dos experimentos oficiais.
 *
 * Para cada cenário e seed, a carga é gerada exatamente uma vez e reutilizada
 * pelos quatro algoritmos. Isso garante uma comparação pareada e justa: todos
 * recebem os mesmos processos, chegadas, prioridades e rajadas de CPU/E/S.
 *
 * A linha do tempo é desativada porque não é necessária para as métricas e
 * consumiria muita memória nas execuções com 1.000 processos.
 */

typedef struct {
    SchedulerAlgorithm kind;
    const char *csv_name;
} AlgorithmEntry;

static const AlgorithmEntry ALGORITHMS[] = {
    {SCHED_FCFS, "fcfs"},
    {SCHED_ROUND_ROBIN, "round_robin"},
    {SCHED_PRIORITY, "prioridade"},
    {SCHED_EPA, "epa"}
};

static const ScenarioKind SCENARIOS[] = {
    SCENARIO_BALANCED,
    SCENARIO_IO_BOUND,
    SCENARIO_CPU_BOUND,
    SCENARIO_UNBALANCED_PRIORITY
};

/* Nomes sem espaços ou acentos simplificam o processamento posterior do CSV. */
static const char *scenario_csv_name(ScenarioKind scenario) {
    switch (scenario) {
    case SCENARIO_BALANCED: return "equilibrado";
    case SCENARIO_IO_BOUND: return "io_bound";
    case SCENARIO_CPU_BOUND: return "cpu_bound";
    case SCENARIO_UNBALANCED_PRIORITY: return "prioridades_desbalanceadas";
    default: return "desconhecido";
    }
}

/* Converte e valida argumentos inteiros positivos da linha de comando. */
static int parse_positive_int(const char *text, const char *name) {
    char *end = NULL;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || !end || *end != '\0' || value <= 0 || value > INT_MAX) {
        fprintf(stderr, "Valor inválido para %s: %s\n", name, text);
        return -1;
    }
    return (int)value;
}

/*
 * Grava uma linha por processo. O analisador calculará turnaround e slowdown
 * usando somente estes dados brutos, sem duplicar a lógica do simulador.
 */
static int write_process_rows(FILE *file, unsigned long long seed,
                              const char *scenario, const char *algorithm,
                              const SimulationResult *result) {
    size_t i;

    for (i = 0; i < result->process_count; ++i) {
        const ProcessResult *process = &result->processes[i];
        if (fprintf(file, "%llu,%s,%s,%d,%d,%d,%d,%d\n",
                    seed, scenario, algorithm, process->id,
                    process->arrival_time, process->completion_time,
                    process->total_cpu_time, process->total_io_time) < 0)
            return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *process_path = argc > 1 ? argv[1] : "processos.csv";
    const char *run_path = argc > 2 ? argv[2] : "execucoes.csv";
    int process_count = argc > 3 ? parse_positive_int(argv[3], "processos") : 1000;
    int seed_count = argc > 4 ? parse_positive_int(argv[4], "seeds") : 100;
    int quantum = argc > 5 ? parse_positive_int(argv[5], "quantum") : 4;
    int context_cost = argc > 6 ? parse_positive_int(argv[6], "custo de troca") : 1;
    FILE *process_file = NULL;
    FILE *run_file = NULL;
    size_t scenario_index, algorithm_index;
    int seed_number;
    int status = EXIT_FAILURE;

    if (process_count <= 0 || seed_count <= 0 || quantum <= 0 || context_cost <= 0) {
        fprintf(stderr,
                "Uso: %s [processos.csv] [execucoes.csv] "
                "[num_processos] [num_seeds] [quantum] [custo_troca]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    process_file = fopen(process_path, "w");
    if (!process_file) {
        perror(process_path);
        goto cleanup;
    }
    run_file = fopen(run_path, "w");
    if (!run_file) {
        perror(run_path);
        goto cleanup;
    }

    fprintf(process_file,
            "seed,cenario,algoritmo,pid,chegada,termino,cpu_total,io_total\n");
    fprintf(run_file, "seed,cenario,algoritmo,trocas_contexto\n");

    for (scenario_index = 0;
         scenario_index < sizeof(SCENARIOS) / sizeof(SCENARIOS[0]);
         ++scenario_index) {
        ScenarioKind scenario = SCENARIOS[scenario_index];
        const char *scenario_name = scenario_csv_name(scenario);
        WorkloadParams params = scenario_params(scenario, (size_t)process_count);

        for (seed_number = 1; seed_number <= seed_count; ++seed_number) {
            Workload workload;
            unsigned long long seed = (unsigned long long)seed_number;

            /* Uma única carga é compartilhada pelos quatro algoritmos. */
            if (workload_generate(seed, &params, &workload) != 0) {
                fprintf(stderr, "Falha ao gerar cenário %s, seed %llu.\n",
                        scenario_name, seed);
                goto cleanup;
            }

            for (algorithm_index = 0;
                 algorithm_index < sizeof(ALGORITHMS) / sizeof(ALGORITHMS[0]);
                 ++algorithm_index) {
                const AlgorithmEntry *algorithm = &ALGORITHMS[algorithm_index];
                SimulationConfig config = {
                    algorithm->kind,
                    quantum,
                    context_cost,
                    0 /* não registrar a linha do tempo em cargas grandes */
                };
                SimulationResult result;

                if (simulator_run(workload.processes, workload.process_count,
                                  &config, &result) != 0) {
                    fprintf(stderr,
                            "Falha na simulação: cenário=%s seed=%llu algoritmo=%s.\n",
                            scenario_name, seed, algorithm->csv_name);
                    workload_destroy(&workload);
                    goto cleanup;
                }

                if (write_process_rows(process_file, seed, scenario_name,
                                       algorithm->csv_name, &result) != 0 ||
                    fprintf(run_file, "%llu,%s,%s,%d\n",
                            seed, scenario_name, algorithm->csv_name,
                            result.context_switches) < 0) {
                    fprintf(stderr, "Falha ao gravar os arquivos CSV.\n");
                    simulator_result_destroy(&result);
                    workload_destroy(&workload);
                    goto cleanup;
                }

                simulator_result_destroy(&result);
            }
            workload_destroy(&workload);

            fprintf(stderr, "Concluído: cenário=%s seed=%d/%d\r",
                    scenario_name, seed_number, seed_count);
        }
        fputc('\n', stderr);
    }

    if (fflush(process_file) != 0 || fflush(run_file) != 0) {
        perror("Falha ao finalizar CSV");
        goto cleanup;
    }
    status = EXIT_SUCCESS;

cleanup:
    if (process_file && fclose(process_file) != 0)
        status = EXIT_FAILURE;
    if (run_file && fclose(run_file) != 0)
        status = EXIT_FAILURE;
    return status;
}