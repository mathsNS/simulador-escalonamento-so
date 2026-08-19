#include "scenarios.h"

const char *scenario_name(ScenarioKind kind) {
    switch (kind) {
    case SCENARIO_BALANCED: return "Aleatório equilibrado";
    case SCENARIO_IO_BOUND: return "I/O-bound";
    case SCENARIO_CPU_BOUND: return "CPU-bound / processos longos";
    case SCENARIO_UNBALANCED_PRIORITY: return "Prioridades desbalanceadas";
    default: return "Desconhecido";
    }
}

WorkloadParams scenario_params(ScenarioKind kind, size_t process_count) {
    WorkloadParams p = {0};
    p.process_count = process_count;
    p.min_priority = 1;
    p.max_priority = 5;

    switch (kind) {
    case SCENARIO_BALANCED:
        /*
         * Mistura de processos curtos e longos, com pouca e muita E/S:
         * faixas largas o suficiente para que ambos os extremos apareçam
         * na mesma carga.
         */
        p.mean_interarrival = 5.0;
        p.min_cpu_burst = 1;  p.max_cpu_burst = 20;
        p.min_io_count = 0;   p.max_io_count = 5;
        p.min_io_burst = 1;   p.max_io_burst = 15;
        break;

    case SCENARIO_IO_BOUND:
        /* Rajadas de CPU curtas e muitas requisições de E/S por processo. */
        p.mean_interarrival = 3.0;
        p.min_cpu_burst = 1;  p.max_cpu_burst = 4;
        p.min_io_count = 3;   p.max_io_count = 8;
        p.min_io_burst = 2;   p.max_io_burst = 12;
        break;

    case SCENARIO_CPU_BOUND:
        /* Rajadas de CPU longas e pouquíssima E/S. */
        p.mean_interarrival = 8.0;
        p.min_cpu_burst = 15; p.max_cpu_burst = 40;
        p.min_io_count = 0;   p.max_io_count = 1;
        p.min_io_burst = 1;   p.max_io_burst = 5;
        break;

    case SCENARIO_UNBALANCED_PRIORITY:
        /*
         * Mesma forma de carga do cenário equilibrado (o eixo variado aqui
         * é a prioridade, não CPU/E-S): 80% dos processos caem na fatia de
         * alta prioridade (1-2) e 20% na de baixa prioridade (3-5).
         */
        p.mean_interarrival = 5.0;
        p.min_cpu_burst = 1;  p.max_cpu_burst = 20;
        p.min_io_count = 0;   p.max_io_count = 5;
        p.min_io_burst = 1;   p.max_io_burst = 15;
        p.high_priority_fraction = 0.8;
        p.high_priority_boundary = 2;
        break;
    }
    return p;
}
