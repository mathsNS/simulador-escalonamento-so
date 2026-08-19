#include "workload.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

void rng_seed(Rng *rng, unsigned long long seed) {
    /* xorshift64* não pode iniciar com estado zero. */
    rng->state = seed ? seed : 0x9E3779B97F4A7C15ULL;
}

static unsigned long long xorshift64star(unsigned long long *state) {
    unsigned long long x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

double rng_uniform01(Rng *rng) {
    /* 53 bits de mantissa: mesma resolução usada por geradores double comuns. */
    unsigned long long bits = xorshift64star(&rng->state) >> 11;
    return (double)bits / (double)(1ULL << 53);
}

void generate_arrivals(Rng *rng, size_t count, double mean_interarrival,
                       int *arrivals_out) {
    size_t i;
    int time = 0;
    for (i = 0; i < count; ++i) {
        if (mean_interarrival > 0.0) {
            double u = rng_uniform01(rng);
            if (u <= 0.0)
                u = 1e-12; /* evita log(0) na amostragem por transformada inversa */
            time += (int)(-mean_interarrival * log(u) + 0.5);
        }
        arrivals_out[i] = time;
    }
}

int rng_uniform_int(Rng *rng, int min, int max) {
    unsigned long long range = (unsigned long long)(max - min) + 1;
    return min + (int)(rng_uniform01(rng) * (double)range);
}

static int validate_params(const WorkloadParams *p) {
    if (!p || p->process_count == 0 || p->mean_interarrival < 0.0)
        return -1;
    if (p->min_priority > p->max_priority)
        return -1;
    if (p->min_cpu_burst < 1 || p->min_cpu_burst > p->max_cpu_burst)
        return -1;
    if (p->min_io_count < 0 || p->min_io_count > p->max_io_count)
        return -1;
    /* Duracao de E/S so precisa ser valida se algum processo puder ter E/S. */
    if (p->max_io_count > 0 &&
        (p->min_io_burst < 1 || p->min_io_burst > p->max_io_burst))
        return -1;
    return 0;
}

void workload_destroy(Workload *workload) {
    if (!workload)
        return;
    free(workload->processes);
    free(workload->bursts_storage);
    memset(workload, 0, sizeof(*workload));
}

int workload_generate(unsigned long long seed, const WorkloadParams *params,
                      Workload *out) {
    Rng rng_struct, rng_arrival;
    size_t *io_counts = NULL;
    int *arrivals = NULL;
    size_t i, j, total_slots = 0, offset = 0;

    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    if (validate_params(params) != 0)
        return -1;

    /*
     * Dois fluxos de aleatoriedade independentes, derivados da mesma seed
     * por XOR com constantes distintas: um para a estrutura do processo
     * (prioridade, número e duração das rajadas) e outro para os tempos de
     * chegada. Isso garante "mesma seed => mesma carga" mesmo se a ordem de
     * geração de um dos dois for alterada no futuro.
     */
    rng_seed(&rng_struct, seed ^ 0xA5A5A5A5A5A5A5A5ULL);
    rng_seed(&rng_arrival, seed ^ 0x5A5A5A5A5A5A5A5AULL);

    io_counts = malloc(params->process_count * sizeof(*io_counts));
    arrivals = malloc(params->process_count * sizeof(*arrivals));
    out->processes = calloc(params->process_count, sizeof(*out->processes));
    if (!io_counts || !arrivals || !out->processes)
        goto fail;

    /* Passo 1: número de requisições de E/S de cada processo, para saber o
     * tamanho total do armazenamento de rajadas antes de alocá-lo. */
    for (i = 0; i < params->process_count; ++i) {
        int io_count = rng_uniform_int(&rng_struct, params->min_io_count,
                                       params->max_io_count);
        io_counts[i] = (size_t)io_count;
        total_slots += 2 * (size_t)io_count + 1;
    }

    out->bursts_storage = malloc(total_slots * sizeof(*out->bursts_storage));
    if (!out->bursts_storage)
        goto fail;

    /* Passo 2: prioridade e durações de rajada (CPU, E/S, CPU, ...),
     * continuando o mesmo fluxo de aleatoriedade do passo 1. */
    for (i = 0; i < params->process_count; ++i) {
        size_t burst_count = 2 * io_counts[i] + 1;
        int *bursts = out->bursts_storage + offset;
        for (j = 0; j < burst_count; ++j)
            bursts[j] = (j % 2 == 0)
                ? rng_uniform_int(&rng_struct, params->min_cpu_burst, params->max_cpu_burst)
                : rng_uniform_int(&rng_struct, params->min_io_burst, params->max_io_burst);

        out->processes[i].id = (int)i + 1;
        out->processes[i].priority = rng_uniform_int(&rng_struct, params->min_priority,
                                                      params->max_priority);
        out->processes[i].bursts = bursts;
        out->processes[i].burst_count = burst_count;
        offset += burst_count;
    }

    /* Tempos de chegada: fluxo próprio, independente do de estrutura. */
    generate_arrivals(&rng_arrival, params->process_count,
                      params->mean_interarrival, arrivals);
    for (i = 0; i < params->process_count; ++i)
        out->processes[i].arrival_time = arrivals[i];

    out->process_count = params->process_count;
    free(io_counts);
    free(arrivals);
    return 0;

fail:
    free(io_counts);
    free(arrivals);
    workload_destroy(out);
    return -1;
}
