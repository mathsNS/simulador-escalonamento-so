#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#include "simulator.h"

/*
 * PCB (Process Control Block): entidade simulada de cada processo.
 *
 * Campos estáticos, definidos na carga de trabalho:
 *   - spec.id            Identificador do processo.
 *   - spec.arrival_time  Tempo de chegada.
 *   - spec.priority      Prioridade (convenção: valor menor = maior prioridade).
 *   - spec.bursts        Lista de rajadas CPU -> E/S -> CPU -> ... (sempre
 *                        começa e termina em CPU); os índices ímpares são as
 *                        durações de cada requisição de E/S.
 *
 * Campos dinâmicos, atualizados durante a simulação:
 *   - state          Estado atual: novo, pronto, executando, bloqueado ou
 *                    finalizado.
 *   - burst_index    Índice da rajada (CPU ou E/S) em execução/aguardando.
 *   - remaining      Tempo restante da rajada atual.
 *   - unblock_time   Instante em que uma E/S em andamento termina.
 *   - completion_time Instante em que o processo termina.
 *   - ready_order    Ordem de entrada na fila de prontos (desempate FIFO).
 *   - total_cpu      Soma de todas as rajadas de CPU do processo.
 *   - total_io       Soma de todas as rajadas de E/S do processo.
 */
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

    /* --- Estado histórico usado pelo EPA (Escalonador Preditivo Adaptativo,
       algoritmo próprio da equipe - ver scheduler_epa.c). Mantido pelo núcleo
       (simulator.c) nos pontos em que cada dado passa a ser conhecido: ao
       concluir uma rajada de CPU, ao iniciar uma rajada de E/S e a cada
       unidade de tempo em que o processo permanece pronto. Os demais
       escalonadores (FCFS, RR, prioridade) ignoram estes campos.
       IMPORTANTE: nada aqui usa `spec.bursts` além do índice já concluído -
       ver a nota "sem conhecimento futuro" em scheduler_epa.c. */
    double epa_predicted_burst;  /* estimativa (EWMA) da próxima rajada de CPU. */
    long long epa_waiting_ticks; /* unidades de tempo consecutivas pronto, sem executar. */
    int epa_observed_cpu_time;   /* soma das rajadas de CPU já concluídas. */
    int epa_observed_io_time;    /* soma das rajadas de E/S já iniciadas/concluídas. */
} PCB;

/* Estimativa inicial de rajada usada pelo EPA antes de haver qualquer rajada
   de CPU observada para o processo (não pode ser derivada de spec.bursts,
   pois isso seria conhecimento futuro). Vive aqui por ser usada tanto pela
   inicialização no núcleo (simulator.c) quanto, potencialmente, por outros
   pontos de instrumentação. */
#define EPA_INITIAL_BURST_ESTIMATE 5.0

typedef int (*ChooseProcess)(const PCB *, size_t);

int scheduler_choose_fcfs(const PCB *processes, size_t count);
int scheduler_choose_priority(const PCB *processes, size_t count);
int scheduler_choose_epa(const PCB *processes, size_t count);

#endif
