#include "simulator.h"

#include <assert.h>
#include <stdio.h>

static void expect_timeline(SchedulerAlgorithm algorithm, int quantum,
                            const ProcessSpec *specs, size_t count,
                            const int *expected, size_t expected_count) {
    SimulationConfig config = {algorithm, quantum, 0, 1};
    SimulationResult result;
    size_t i;
    assert(simulator_run(specs, count, &config, &result) == 0);
    assert(result.timeline_length == expected_count);
    for (i = 0; i < expected_count; ++i)
        assert(result.timeline[i] == expected[i]);
    simulator_result_destroy(&result);
}

static void test_fcfs_arrivals_and_idle(void) {
    const int a[] = {2}; const int b[] = {1};
    const ProcessSpec p[] = {{1, 2, 1, a, 1}, {2, 2, 1, b, 1}};
    const int expected[] = {SIM_IDLE, SIM_IDLE, 1, 1, 2};
    expect_timeline(SCHED_FCFS, 1, p, 2, expected, 5);
}

static void test_round_robin_quantum_and_short_burst(void) {
    const int a[] = {5}; const int b[] = {1};
    const ProcessSpec p[] = {{1, 0, 1, a, 1}, {2, 0, 1, b, 1}};
    const int expected[] = {1, 1, 2, 1, 1, 1};
    expect_timeline(SCHED_ROUND_ROBIN, 2, p, 2, expected, 6);
}

static void test_priority_non_preemptive_and_tie(void) {
    const int a[] = {3}; const int b[] = {1}; const int c[] = {1};
    const ProcessSpec p[] = {
        {1, 0, 5, a, 1}, {3, 1, 1, c, 1}, {2, 1, 1, b, 1}
    };
    /* P3 aparece antes na carga e, portanto, entra antes na fila no empate. */
    const int expected[] = {1, 1, 1, 3, 2};
    expect_timeline(SCHED_PRIORITY, 1, p, 3, expected, 5);
}

static void test_io_return(void) {
    const int a[] = {1, 2, 1}; const int b[] = {3};
    const ProcessSpec p[] = {{1, 0, 1, a, 3}, {2, 0, 1, b, 1}};
    const int expected[] = {1, 2, 2, 2, 1};
    expect_timeline(SCHED_FCFS, 1, p, 2, expected, 5);
}

static void test_round_robin_io_before_quantum(void) {
    const int a[] = {1, 2, 1}; const int b[] = {3};
    const ProcessSpec p[] = {{1, 0, 1, a, 3}, {2, 0, 1, b, 1}};
    const int expected[] = {1, 2, 2, 2, 1};
    expect_timeline(SCHED_ROUND_ROBIN, 5, p, 2, expected, 5);
}

static void test_context_switch_cost(void) {
    const int a[] = {1}; const int b[] = {1};
    const ProcessSpec p[] = {{1, 0, 1, a, 1}, {2, 0, 1, b, 1}};
    const int expected[] = {1, SIM_CONTEXT_SWITCH, SIM_CONTEXT_SWITCH, 2};
    SimulationConfig config = {SCHED_FCFS, 1, 2, 1};
    SimulationResult result;
    assert(simulator_run(p, 2, &config, &result) == 0);
    assert(result.timeline_length == 4);
    for (size_t i = 0; i < 4; ++i)
        assert(result.timeline[i] == expected[i]);
    assert(result.context_switches == 1);
    simulator_result_destroy(&result);
}

static void test_invalid_quantum(void) {
    const int burst[] = {1};
    const ProcessSpec p = {1, 0, 1, burst, 1};
    const SimulationConfig config = {SCHED_ROUND_ROBIN, 0, 0, 0};
    SimulationResult result;
    assert(simulator_run(&p, 1, &config, &result) == -1);
}

int main(void) {
    puts("FCFS: chegadas e ociosidade"); fflush(stdout);
    test_fcfs_arrivals_and_idle();
    puts("Round Robin: quantum"); fflush(stdout);
    test_round_robin_quantum_and_short_burst();
    puts("Prioridade: não preemptivo e empate"); fflush(stdout);
    test_priority_non_preemptive_and_tie();
    puts("FCFS: retorno de E/S"); fflush(stdout);
    test_io_return();
    puts("Round Robin: E/S antes do quantum"); fflush(stdout);
    test_round_robin_io_before_quantum();
    puts("Troca de contexto"); fflush(stdout);
    test_context_switch_cost();
    puts("Validação de quantum"); fflush(stdout);
    test_invalid_quantum();
    puts("Todos os testes passaram.");
    return 0;
}
