#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stddef.h>

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

#endif
