#include "scheduler_internal.h"

/*
 * O EPA é não preemptivo e escolhe o menor score:
 *
 *   previsão - envelhecimento - afinidade_com_E/S + prioridade
 *
 * A previsão usa uma média móvel das rajadas já concluídas. O envelhecimento
 * reduz o risco de inanição, enquanto a afinidade favorece processos com mais
 * E/S observada. A duração real da rajada atual ou de rajadas futuras não
 * participa da decisão. Empates seguem a ordem de entrada na fila e o PID.
 */
#define EPA_AGING_WEIGHT 1.0
#define EPA_IO_WEIGHT 3.0
#define EPA_PRIORITY_WEIGHT 0.5

int scheduler_choose_epa(const PCB *processes, size_t count) {
    int best = -1;
    double best_score = 0.0;
    size_t i;

    for (i = 0; i < count; ++i) {
        double cpu_total, io_total, io_affinity, score;

        if (processes[i].state != PROCESS_READY)
            continue;

        cpu_total = (double)processes[i].epa_observed_cpu_time;
        io_total = (double)processes[i].epa_observed_io_time;
        io_affinity = (cpu_total + io_total > 0.0)
            ? io_total / (cpu_total + io_total)
            : 0.0;

        score = processes[i].epa_predicted_burst
              - EPA_AGING_WEIGHT * (double)processes[i].epa_waiting_ticks
              - EPA_IO_WEIGHT * io_affinity
              + EPA_PRIORITY_WEIGHT * (double)processes[i].spec.priority;

        if (best < 0 || score < best_score ||
            (score == best_score &&
             (processes[i].ready_order < processes[best].ready_order ||
              (processes[i].ready_order == processes[best].ready_order &&
               processes[i].spec.id < processes[best].spec.id)))) {
            best = (int)i;
            best_score = score;
        }
    }
    return best;
}
