#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stddef.h>

#include "simulator.h"

/*
 * Gerador pseudoaleatório determinístico (xorshift64*), com estado próprio
 * por instância. Não usa rand()/srand() da libc: assim, gerar vários
 * cenários/seeds no mesmo processo nunca faz uma sequência 
 * interferir na outra
 */
typedef struct {
    unsigned long long state;
} Rng;

/* Inicializa o gerador a partir de uma seed (0 é uma seed válida). */
void rng_seed(Rng *rng, unsigned long long seed);

/* Amostra uniforme em [0, 1). */
double rng_uniform01(Rng *rng);

/*
 * Modelo de chegada de processos: processo de Poisson, ou seja, os 
 * intervalos entre chegadas consecutivas seguem uma distribuição 
 * exponencial de média mean_interarrival (em unidades de tempo de simulação). 
 * Preenche arrivals_out[0..count-1] com os tempos de chegada dos `count` 
 * processos, em ordem não decrescente, comecando em t >= 0.
 *
 * Caso especial: mean_interarrival == 0 faz todos os processos chegarem no
 * instante 0 (lote único).
 */
void generate_arrivals(Rng *rng, size_t count, double mean_interarrival,
                       int *arrivals_out);

/* Amostra inteira uniforme em [min, max] (ambos inclusive). */
int rng_uniform_int(Rng *rng, int min, int max);

/*
 * Parâmetros da geração de carga. 
 * Todas as faixas sao inclusivas nos dois extremos.
 */
typedef struct {
    size_t process_count;
    double mean_interarrival;   /* ver generate_arrivals(). */
    int min_priority, max_priority;
    int min_cpu_burst, max_cpu_burst; /* duração de cada rajada de CPU (> 0). */
    int min_io_count, max_io_count;   /* requisições de E/S por processo (>= 0). */
    int min_io_burst, max_io_burst;   /* duração de cada requisição de E/S (> 0). */
} WorkloadParams;

/*
 * Carga de processos gerada por workload_generate(). Cada
 * processes[i].bursts aponta para dentro de bursts_storage (um único bloco
 * alocado para todas as rajadas de todos os processos); libera tudo de uma
 * vez com workload_destroy().
 */
typedef struct {
    ProcessSpec *processes;
    int *bursts_storage;
    size_t process_count;
} Workload;

/*
 * Gera params->process_count processos deterministicamente a partir de
 * seed: mesma seed + mesmos params sempre produzem a mesma carga. 
 * IDs recebem 1..process_count. A ordem de amostragem por processo é: 
 * número de requisições de E/S, durações das rajadas (CPU, E/S, CPU, ...) e
 * prioridade; os tempos de chegada usam um fluxo de aleatoriedade próprio,
 * independente desse.
 *
 * Retorna 0 em sucesso e -1 se os parametros forem inválidos ou a alocação
 * falhar (nesse caso *out fica zerado).
 */
int workload_generate(unsigned long long seed, const WorkloadParams *params,
                      Workload *out);
void workload_destroy(Workload *workload);

#endif
