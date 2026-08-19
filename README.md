# Simulador de escalonamento de processos

Simulador discreto em C para comparar quatro políticas de escalonamento:

- FCFS;
- Round Robin;
- Prioridade não preemptiva;
- EPA (Escalonador Preditivo Adaptativo).

Os processos possuem tempo de chegada, prioridade e uma sequência alternada de
rajadas de CPU e E/S. O simulador também considera bloqueios, retornos à fila de
prontos, preempção e custo de troca de contexto.

## Requisitos

- compilador compatível com C11;
- `make`;
- Python 3 para métricas, gráficos e testes da análise.

## Compilação e execução

```sh
make
make test
```

Execute a carga demonstrativa com uma das políticas:

```sh
./simulator fcfs
./simulator rr 2 1
./simulator priority 2 1
./simulator epa
```

O formato é:

```text
./simulator ALGORITMO QUANTUM CUSTO_TROCA
```

O quantum é utilizado apenas pelo Round Robin. Na linha do tempo, um PID
representa execução, `cs` representa troca de contexto e `idle`, CPU ociosa.

Para remover os executáveis gerados:

```sh
make clean
```

## Estrutura do repositório

```text
include/       interfaces públicas e estruturas internas
src/           núcleo, escalonadores, cargas e executor experimental
tests/         testes em C
analysis/      cálculo de métricas, IC95% e gráficos
docs/          documentação técnica
resultados/    resultados consolidados dos experimentos
```

## Modelo da simulação

- O tempo avança em unidades discretas.
- Rajadas seguem a ordem `CPU -> E/S -> CPU -> ... -> CPU`.
- A fila de prontos é formada pelos PCBs em `PROCESS_READY`.
- Operações de E/S de processos diferentes ocorrem em paralelo, sem fila de
  dispositivo.
- Chegadas são processadas antes de conclusões de E/S no mesmo instante.
- Valores numéricos menores representam prioridades maiores.
- A primeira execução e a retomada após ociosidade não contam como troca de
  contexto.
- Durante uma troca, a CPU fica indisponível, mas chegadas e E/S continuam
  avançando.

Os estados possíveis são `NEW`, `READY`, `RUNNING`, `BLOCKED` e `FINISHED`.
FCFS e Round Robin usam a ordem de entrada na fila. Na Prioridade, empates são
resolvidos por entrada na fila, chegada e PID.

## Algoritmos

### FCFS

Executa os processos pela ordem de entrada na fila de prontos. É não
preemptivo durante uma rajada de CPU.

### Round Robin

Executa cada processo por até um quantum. Se a rajada não terminar, o processo
volta ao fim da fila de prontos.

### Prioridade não preemptiva

Escolhe o processo pronto com o menor valor numérico de prioridade. A chegada
de um processo mais prioritário não interrompe uma rajada em andamento.

### EPA: Escalonador Preditivo Adaptativo

O EPA é a política proposta no projeto. Ele combina previsão da próxima rajada,
tempo de espera, afinidade com E/S e prioridade estática em uma pontuação com
critérios normalizados. A decisão usa apenas informações já observadas e é não
preemptiva por rajada.

A documentação completa do EPA está em
[docs/algoritmo_epa.md](docs/algoritmo_epa.md). O documento apresenta a fórmula,
os pesos, a atualização da previsão, um exemplo numérico, os desempates, a
complexidade e as limitações do algoritmo.

## Cargas e cenários

As cargas são geradas deterministicamente por seed com `workload_generate`.
Tempos de chegada seguem intervalos exponenciais com média configurável; média
zero coloca todos os processos no instante inicial.

| Cenário | CPU | Requisições de E/S | Prioridade |
|---|---:|---:|---|
| Equilibrado | 1 a 20 | 0 a 5 | uniforme de 1 a 5 |
| I/O-bound | 1 a 4 | 3 a 8 | uniforme de 1 a 5 |
| CPU-bound | 15 a 40 | 0 a 1 | uniforme de 1 a 5 |
| Prioridades desbalanceadas | 1 a 20 | 0 a 5 | 80% entre 1 e 2 |

## Experimentos e métricas

Compile o executor:

```sh
make experiment
```

Execute a configuração mínima utilizada no projeto:

```sh
./experiment dados/processos.csv dados/execucoes.csv 1000 100 4 1
```

Os argumentos finais representam, respectivamente, número de processos,
número de seeds, quantum e custo de troca. A execução avalia os quatro
algoritmos nos quatro cenários usando as mesmas cargas.

Calcule as métricas e gere os gráficos:

```sh
python analysis/analisar.py --processos dados/processos.csv --execucoes dados/execucoes.csv --saida resultados
python analysis/comparar_resultados.py --resumo resultados/resumo_ic95.csv
```

São produzidos turnaround médio, trocas de contexto, slowdown médio, índice de
Jain do slowdown e IC95%. Os CSVs registram seed, cenário, algoritmo, quantum,
custo de troca e tamanho da carga.

## Documentação e resultados

Os resultados versionados correspondem ao EPA com critérios normalizados e à
configuração de 1.000 processos e 100 seeds descrita na metodologia.

- [Descrição completa do EPA](docs/algoritmo_epa.md)
- [Metodologia experimental](docs/metodologia_experimental.md)
- [Análise comparativa](resultados/analise_comparativa.md)
- [Configuração dos experimentos](resultados/configuracao_experimental.csv)
- [Resumo com IC95%](resultados/resumo_ic95.csv)

Os gráficos consolidados estão em `resultados/graficos/`.
