#include "scheduler_internal.h"

/*
 * O EPA é não preemptivo e escolhe o menor score:
 *
 *   previsão - espera - afinidade_com_E/S + prioridade
 *
 * Os critérios são normalizados entre os processos prontos para impedir que a
 * unidade de uma medida domine as demais. A duração real da rajada atual ou de
 * rajadas futuras não participa da decisão. Empates seguem a ordem de entrada
 * na fila e o PID.
 */
#define EPA_PREDICTION_WEIGHT 0.40
#define EPA_WAITING_WEIGHT 0.30
#define EPA_IO_WEIGHT 0.20
#define EPA_PRIORITY_WEIGHT 0.10
#define EPA_SCORE_EPSILON 1e-12

static double normalized_double(double value, double minimum, double maximum) {
    return maximum > minimum ? (value - minimum) / (maximum - minimum) : 0.0;
}

static double normalized_int(int value, int minimum, int maximum) {
    return maximum > minimum
        ? (double)(value - minimum) / (double)(maximum - minimum)
        : 0.0;
}

static int scores_are_equal(double first, double second) {
    double difference = first - second;
    if (difference < 0.0)
        difference = -difference;
    return difference <= EPA_SCORE_EPSILON;
}

int scheduler_choose_epa(const PCB *processes, size_t count) {
    int best = -1;
    double best_score = 0.0;
    double min_prediction = 0.0, max_prediction = 0.0;
    long long max_waiting = 0;
    int min_priority = 0, max_priority = 0;
    size_t i;

    for (i = 0; i < count; ++i) {
        if (processes[i].state != PROCESS_READY)
            continue;

        if (best < 0) {
            best = (int)i;
            min_prediction = max_prediction = processes[i].epa_predicted_burst;
            max_waiting = processes[i].epa_waiting_ticks;
            min_priority = max_priority = processes[i].spec.priority;
        } else {
            if (processes[i].epa_predicted_burst < min_prediction)
                min_prediction = processes[i].epa_predicted_burst;
            if (processes[i].epa_predicted_burst > max_prediction)
                max_prediction = processes[i].epa_predicted_burst;
            if (processes[i].epa_waiting_ticks > max_waiting)
                max_waiting = processes[i].epa_waiting_ticks;
            if (processes[i].spec.priority < min_priority)
                min_priority = processes[i].spec.priority;
            if (processes[i].spec.priority > max_priority)
                max_priority = processes[i].spec.priority;
        }
    }

    if (best < 0)
        return -1;

    best = -1;

    for (i = 0; i < count; ++i) {
        double cpu_total, io_total, io_affinity;
        double prediction, waiting, priority, score;

        if (processes[i].state != PROCESS_READY)
            continue;

        cpu_total = (double)processes[i].epa_observed_cpu_time;
        io_total = (double)processes[i].epa_observed_io_time;
        io_affinity = (cpu_total + io_total > 0.0)
            ? io_total / (cpu_total + io_total)
            : 0.0;

        prediction = normalized_double(processes[i].epa_predicted_burst,
                                       min_prediction, max_prediction);
        waiting = max_waiting > 0
            ? (double)processes[i].epa_waiting_ticks / (double)max_waiting
            : 0.0;
        priority = normalized_int(processes[i].spec.priority,
                                  min_priority, max_priority);

        score = EPA_PREDICTION_WEIGHT * prediction
              - EPA_WAITING_WEIGHT * waiting
              - EPA_IO_WEIGHT * io_affinity
              + EPA_PRIORITY_WEIGHT * priority;

        if (best < 0 || score < best_score - EPA_SCORE_EPSILON ||
            (scores_are_equal(score, best_score) &&
             (processes[i].ready_order < processes[best].ready_order ||
              (processes[i].ready_order == processes[best].ready_order &&
               processes[i].spec.id < processes[best].spec.id)))) {
            best = (int)i;
            best_score = score;
        }
    }
    return best;
}
