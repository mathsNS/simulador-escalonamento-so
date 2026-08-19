#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#include "simulator.h"

typedef struct {
    ProcessSpec spec;
    ProcessState state;
    size_t burst_index;
    int remaining;
    int unblock_time;
    int completion_time;
    unsigned long long ready_order;
    int total_cpu;
    int total_io;

    /* --- Estado historico usado pelo EPA (Escalonador Preditivo Adaptativo,
       algoritmo proprio da equipe - ver scheduler_epa.c). Mantido pelo nucleo
       (simulator.c) nos pontos em que cada dado passa a ser conhecido: ao
       concluir uma rajada de CPU, ao iniciar uma rajada de E/S e a cada
       unidade de tempo em que o processo permanece pronto. Os demais
       escalonadores (FCFS, RR, prioridade) ignoram estes campos.
       IMPORTANTE: nada aqui usa `spec.bursts` alem do indice ja concluido -
       ver a nota "sem conhecimento futuro" em scheduler_epa.c. */
    double epa_predicted_burst;  /* estimativa (EWMA) da proxima rajada de CPU. */
    long long epa_waiting_ticks; /* unidades de tempo consecutivas pronto, sem executar. */
    int epa_observed_cpu_time;   /* soma das rajadas de CPU ja concluidas. */
    int epa_observed_io_time;    /* soma das rajadas de E/S ja iniciadas/concluidas. */
} RuntimeProcess;

/* Estimativa inicial de rajada usada pelo EPA antes de haver qualquer rajada
   de CPU observada para o processo (nao pode ser derivada de spec.bursts,
   pois isso seria conhecimento futuro). Vive aqui por ser usada tanto pela
   inicializacao no nucleo (simulator.c) quanto, potencialmente, por outros
   pontos de instrumentacao. */
#define EPA_INITIAL_BURST_ESTIMATE 5.0

typedef int (*ChooseProcess)(const RuntimeProcess *, size_t);

int scheduler_choose_fcfs(const RuntimeProcess *processes, size_t count);
int scheduler_choose_priority(const RuntimeProcess *processes, size_t count);
int scheduler_choose_epa(const RuntimeProcess *processes, size_t count);

#endif
