# Metodologia experimental

Os experimentos avaliam FCFS, Round Robin, Prioridade e EPA nos cenários
equilibrado, I/O-bound, CPU-bound e prioridades desbalanceadas. Para cada
cenário e seed, uma única carga é gerada e reutilizada pelos quatro algoritmos.
Os resultados versionados usam 1.000 processos, 100 seeds, quantum 4 e custo de
troca de contexto 1.

Cada seed representa uma repetição independente. As métricas são calculadas
primeiro por execução e depois agregadas entre seeds; processos de uma mesma
carga não são tratados como amostras independentes no cálculo do IC95%.

## Métricas

Para cada processo `i`:

- `turnaround_i = termino_i - chegada_i`;
- `tempo_ideal_i = soma_cpu_i + soma_io_i`;
- `slowdown_i = turnaround_i / tempo_ideal_i`.

Para uma execução com `n` processos:

- turnaround médio: média dos turnarounds;
- trocas de contexto: total registrado pelo simulador;
- Jain do slowdown: `(soma(slowdown)^2 / (n * soma(slowdown^2))) * 100`.

Para cada combinação de cenário e algoritmo, o intervalo de confiança é:

`IC95% = media +/- 1,96 * desvio_padrao_amostral / sqrt(numero_de_seeds)`.

O quantum, o custo de troca, o número de processos e o total de seeds são
registrados junto aos resultados. A sobreposição de intervalos de confiança é
apresentada como auxílio visual, não como teste formal de significância.
