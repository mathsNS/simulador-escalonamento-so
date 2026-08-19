#include "workload.h"

#include <assert.h>
#include <stdio.h>

static void test_same_seed_same_workload(void) {
    int a[50], b[50];
    Rng rng_a, rng_b;
    size_t i;
    rng_seed(&rng_a, 42);
    rng_seed(&rng_b, 42);
    generate_arrivals(&rng_a, 50, 10.0, a);
    generate_arrivals(&rng_b, 50, 10.0, b);
    for (i = 0; i < 50; ++i)
        assert(a[i] == b[i]);
}

static void test_different_seed_different_workload(void) {
    int a[50], b[50];
    Rng rng_a, rng_b;
    size_t i;
    int differs = 0;
    rng_seed(&rng_a, 1);
    rng_seed(&rng_b, 2);
    generate_arrivals(&rng_a, 50, 10.0, a);
    generate_arrivals(&rng_b, 50, 10.0, b);
    for (i = 0; i < 50; ++i)
        if (a[i] != b[i])
            differs = 1;
    assert(differs);
}

static void test_arrivals_non_decreasing(void) {
    int a[1000];
    Rng rng;
    size_t i;
    rng_seed(&rng, 7);
    generate_arrivals(&rng, 1000, 3.0, a);
    assert(a[0] >= 0);
    for (i = 1; i < 1000; ++i)
        assert(a[i] >= a[i - 1]);
}

static void test_zero_mean_arrives_at_instant_zero(void) {
    int a[10];
    Rng rng;
    size_t i;
    rng_seed(&rng, 99);
    generate_arrivals(&rng, 10, 0.0, a);
    for (i = 0; i < 10; ++i)
        assert(a[i] == 0);
}

static WorkloadParams sample_params(void) {
    WorkloadParams params = {0};
    params.process_count = 1000;
    params.mean_interarrival = 5.0;
    params.min_priority = 1;
    params.max_priority = 5;
    params.min_cpu_burst = 1;
    params.max_cpu_burst = 10;
    params.min_io_count = 0;
    params.max_io_count = 3;
    params.min_io_burst = 1;
    params.max_io_burst = 8;
    return params;
}

static void test_workload_same_seed_same_workload(void) {
    WorkloadParams params = sample_params();
    Workload a, b;
    size_t i, j;
    assert(workload_generate(12345, &params, &a) == 0);
    assert(workload_generate(12345, &params, &b) == 0);
    assert(a.process_count == b.process_count);
    for (i = 0; i < a.process_count; ++i) {
        assert(a.processes[i].id == b.processes[i].id);
        assert(a.processes[i].arrival_time == b.processes[i].arrival_time);
        assert(a.processes[i].priority == b.processes[i].priority);
        assert(a.processes[i].burst_count == b.processes[i].burst_count);
        for (j = 0; j < a.processes[i].burst_count; ++j)
            assert(a.processes[i].bursts[j] == b.processes[i].bursts[j]);
    }
    workload_destroy(&a);
    workload_destroy(&b);
}

static void test_workload_different_seed_different_workload(void) {
    WorkloadParams params = sample_params();
    Workload a, b;
    size_t i;
    int differs = 0;
    assert(workload_generate(1, &params, &a) == 0);
    assert(workload_generate(2, &params, &b) == 0);
    for (i = 0; i < a.process_count; ++i)
        if (a.processes[i].priority != b.processes[i].priority ||
            a.processes[i].arrival_time != b.processes[i].arrival_time ||
            a.processes[i].burst_count != b.processes[i].burst_count)
            differs = 1;
    assert(differs);
    workload_destroy(&a);
    workload_destroy(&b);
}

static void test_workload_respects_ranges_and_ids(void) {
    WorkloadParams params = sample_params();
    Workload w;
    size_t i, j;
    assert(workload_generate(7, &params, &w) == 0);
    for (i = 0; i < w.process_count; ++i) {
        const ProcessSpec *p = &w.processes[i];
        assert(p->id == (int)i + 1);
        assert(p->arrival_time >= 0);
        assert(p->priority >= params.min_priority && p->priority <= params.max_priority);
        assert(p->burst_count % 2 == 1);
        assert((p->burst_count - 1) / 2 >= (size_t)params.min_io_count &&
               (p->burst_count - 1) / 2 <= (size_t)params.max_io_count);
        for (j = 0; j < p->burst_count; ++j) {
            if (j % 2 == 0)
                assert(p->bursts[j] >= params.min_cpu_burst && p->bursts[j] <= params.max_cpu_burst);
            else
                assert(p->bursts[j] >= params.min_io_burst && p->bursts[j] <= params.max_io_burst);
        }
    }
    workload_destroy(&w);
}

static void test_workload_feeds_simulator(void) {
    WorkloadParams params = sample_params();
    Workload w;
    SimulationConfig config = {SCHED_ROUND_ROBIN, 4, 1, 0};
    SimulationResult result;
    assert(workload_generate(2024, &params, &w) == 0);
    assert(simulator_run(w.processes, w.process_count, &config, &result) == 0);
    assert(result.process_count == w.process_count);
    simulator_result_destroy(&result);
    workload_destroy(&w);
}

static void test_workload_rejects_invalid_params(void) {
    WorkloadParams params = sample_params();
    Workload w;
    params.min_priority = 5;
    params.max_priority = 1; /* faixa invertida */
    assert(workload_generate(1, &params, &w) == -1);
    assert(w.process_count == 0 && w.processes == NULL && w.bursts_storage == NULL);
}

int main(void) {
    puts("Workload: mesma seed produz a mesma carga"); fflush(stdout);
    test_same_seed_same_workload();
    puts("Workload: seeds diferentes produzem cargas diferentes"); fflush(stdout);
    test_different_seed_different_workload();
    puts("Workload: chegadas nao decrescentes"); fflush(stdout);
    test_arrivals_non_decreasing();
    puts("Workload: media zero chega toda no instante 0"); fflush(stdout);
    test_zero_mean_arrives_at_instant_zero();
    puts("Carga completa: mesma seed produz a mesma carga"); fflush(stdout);
    test_workload_same_seed_same_workload();
    puts("Carga completa: seeds diferentes produzem cargas diferentes"); fflush(stdout);
    test_workload_different_seed_different_workload();
    puts("Carga completa: faixas de parametros e IDs respeitados"); fflush(stdout);
    test_workload_respects_ranges_and_ids();
    puts("Carga completa: integra com o simulador"); fflush(stdout);
    test_workload_feeds_simulator();
    puts("Carga completa: parametros invalidos sao rejeitados"); fflush(stdout);
    test_workload_rejects_invalid_params();
    puts("Todos os testes passaram.");
    return 0;
}
