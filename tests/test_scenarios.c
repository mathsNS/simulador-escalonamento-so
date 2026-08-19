#include "scenarios.h"

#include <assert.h>
#include <stdio.h>

static double average_cpu_burst(const Workload *w) {
    double sum = 0.0;
    size_t n = 0, i, j;
    for (i = 0; i < w->process_count; ++i)
        for (j = 0; j < w->processes[i].burst_count; j += 2) {
            sum += w->processes[i].bursts[j];
            ++n;
        }
    return sum / (double)n;
}

static double average_io_count(const Workload *w) {
    double sum = 0.0;
    size_t i;
    for (i = 0; i < w->process_count; ++i)
        sum += (double)(w->processes[i].burst_count - 1) / 2.0;
    return sum / (double)w->process_count;
}

static void test_all_scenarios_generate_valid_workloads(void) {
    ScenarioKind kinds[] = {SCENARIO_BALANCED, SCENARIO_IO_BOUND,
                            SCENARIO_CPU_BOUND, SCENARIO_UNBALANCED_PRIORITY};
    size_t k;
    for (k = 0; k < 4; ++k) {
        WorkloadParams params = scenario_params(kinds[k], 1000);
        Workload w;
        assert(workload_generate(1, &params, &w) == 0);
        assert(w.process_count == 1000);
        workload_destroy(&w);
    }
}

static void test_same_seed_same_scenario_is_reproducible(void) {
    WorkloadParams params = scenario_params(SCENARIO_BALANCED, 500);
    Workload a, b;
    size_t i;
    assert(workload_generate(2026, &params, &a) == 0);
    assert(workload_generate(2026, &params, &b) == 0);
    for (i = 0; i < a.process_count; ++i) {
        assert(a.processes[i].priority == b.processes[i].priority);
        assert(a.processes[i].arrival_time == b.processes[i].arrival_time);
        assert(a.processes[i].burst_count == b.processes[i].burst_count);
    }
    workload_destroy(&a);
    workload_destroy(&b);
}

static void test_io_bound_has_more_io_than_cpu_bound(void) {
    WorkloadParams io_params = scenario_params(SCENARIO_IO_BOUND, 1000);
    WorkloadParams cpu_params = scenario_params(SCENARIO_CPU_BOUND, 1000);
    Workload io_w, cpu_w;
    assert(workload_generate(3, &io_params, &io_w) == 0);
    assert(workload_generate(3, &cpu_params, &cpu_w) == 0);
    /* As faixas de min_io_count/max_io_count de cada cenário não se
     * sobrepõem, então a desigualdade vale para qualquer seed. */
    assert(average_io_count(&io_w) > average_io_count(&cpu_w));
    assert(average_cpu_burst(&io_w) < average_cpu_burst(&cpu_w));
    workload_destroy(&io_w);
    workload_destroy(&cpu_w);
}

static void test_unbalanced_priority_favors_high_priority(void) {
    WorkloadParams params = scenario_params(SCENARIO_UNBALANCED_PRIORITY, 2000);
    Workload w;
    size_t i, high = 0, low = 0;
    assert(workload_generate(4, &params, &w) == 0);
    for (i = 0; i < w.process_count; ++i) {
        if (w.processes[i].priority <= params.high_priority_boundary)
            ++high;
        else
            ++low;
    }
    /* 80%/20% esperado; com n=2000 a folga é enorme, não há chance
     * realista de a maioria virar baixa prioridade. */
    assert(high > low);
    workload_destroy(&w);
}

int main(void) {
    puts("Cenarios: todos geram cargas validas"); fflush(stdout);
    test_all_scenarios_generate_valid_workloads();
    puts("Cenarios: mesma seed reproduz o mesmo cenario"); fflush(stdout);
    test_same_seed_same_scenario_is_reproducible();
    puts("Cenarios: I/O-bound tem mais E/S e menos CPU que CPU-bound"); fflush(stdout);
    test_io_bound_has_more_io_than_cpu_bound();
    puts("Cenarios: prioridades desbalanceadas favorece alta prioridade"); fflush(stdout);
    test_unbalanced_priority_favors_high_priority();
    puts("Todos os testes passaram.");
    return 0;
}
