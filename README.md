# Simulador de escalonamento de processos

Simulador discreto em C dos algoritmos clássicos FCFS, Round Robin e prioridade
não preemptiva, além do algoritmo próprio da equipe (EPA). Os processos são
descritos por rajadas alternadas `CPU, E/S, CPU, ..., CPU`.

## Compilação e execução

```sh
make
./simulator fcfs
./simulator rr 2 1
./simulator priority 2 1
./simulator epa
make test
```

Os argumentos da demonstração são algoritmo, quantum e custo da troca de
contexto. O quantum só é usado pelo Round Robin. Para integrar outra carga,
use a API pública em `include/simulator.h`.

## Fila de prontos

Não há uma estrutura de fila separada: a fila de prontos é o conjunto
implícito de todos os processos com `state == PROCESS_READY`. Cada processo
guarda em `ready_order` um contador monotônico atribuído no instante em que
entrou nesse estado. Os escalonadores (`scheduler_choose_*`) percorrem os
PCBs e só consideram os que estão `READY`; `ready_order` define a ordem FIFO
usada por FCFS e Round Robin, e serve como critério de desempate na
Prioridade. Processos `NEW` (ainda não chegaram) e `BLOCKED` (em E/S) nunca
são elegíveis para a CPU.

## Modelo de chegada de processos

Os tempos de chegada são gerados a partir de uma seed (`include/workload.h`),
com um gerador pseudoaleatório próprio (`Rng`, xorshift64\*) - não a função
`rand()` da libc - para garantir que múltiplos cenários/seeds gerados no
mesmo processo nunca interfiram uns nos outros e que "mesma seed, mesmo
cenário" sempre produza exatamente a mesma carga.

- **Modelo escolhido: intervalos aleatórios (processo de Poisson).** Os
  intervalos entre chegadas consecutivas seguem uma distribuição exponencial
  de média configurável (`mean_interarrival`), amostrada por transformada
  inversa: `intervalo = -media * ln(U)`, com `U` uniforme em `(0, 1)`. Foi
  preferido a intervalos fixos e a "tudo no instante 0" por ser o modelo mais
  realista e usual na literatura de escalonamento, evitando tanto a rigidez
  de chegadas periódicas quanto a ausência de fila inicial.
- Chegadas são não decrescentes e começam em `t >= 0`; a série é determinística
  para uma dada seed (`generate_arrivals`, testado em `tests/test_workload.c`).
- **Caso especial:** `mean_interarrival == 0` faz todos os processos chegarem no
  instante 0 (lote único) - suportado pela função, mas só deve ser usado como
  cenário complementar, com a limitação discutida no relatório.

## Geração de cargas controlada por seed (reprodutibilidade)

`workload_generate(seed, params, &workload)` (`include/workload.h`) gera a
carga completa - ID, prioridade, rajadas de CPU/E-S e tempo de chegada de
cada processo - a partir de uma seed e de faixas de parâmetros
(`WorkloadParams`): **mesma seed + mesmos parâmetros ⇒ sempre a mesma carga**, em qualquer máquina.

- **Determinismo:** não depende de `rand()`/`srand()` (estado global da
  libc); usa dois fluxos do gerador `Rng` próprio, derivados da seed por XOR
  com constantes distintas - um para a estrutura do processo (prioridade,
  número e duração das rajadas) e outro para os tempos de chegada
  (`generate_arrivals`). Assim, gerar vários cenários/seeds no mesmo processo
  não faz uma sequência interferir na outra.
- **Ordem de amostragem por processo** (determinística e fixa): número de
  requisições de E/S → duração de cada rajada (CPU, E/S, CPU, ...) →
  prioridade. Os tempos de chegada vêm de um fluxo separado, aplicado depois.
- **IDs:** atribuídos sequencialmente, `1..process_count`.
- Cada campo é amostrado uniformemente dentro da faixa `[min, max]` inclusiva
  configurada em `WorkloadParams` (prioridade, duração de rajada de CPU,
  quantidade de requisições de E/S, duração de cada requisição de E/S); o
  tempo de chegada segue o modelo de Poisson descrito acima.
- `workload_generate` valida os parâmetros (faixas coerentes, valores
  positivos onde a especificação exige) e retorna `-1` em caso de parâmetro
  inválido ou falha de alocação, sem deixar memória pendurada.
- A carga gerada aponta diretamente para `ProcessSpec`, podendo ser passada
  sem cópia para `simulator_run`; libere tudo com `workload_destroy`.
- Testado em `tests/test_workload.c`: reprodutibilidade (mesma seed → mesma
  carga byte a byte), sensibilidade à seed, respeito às faixas configuradas,
  integração completa com `simulator_run` e rejeição de parâmetros inválidos.

## Cenários obrigatórios

`scenario_params(kind, process_count)` (`include/scenarios.h`) traduz cada um
dos 4 cenários da seção 6 em um preset fixo de `WorkloadParams`; o mesmo
preset (exceto a seed) é usado por todos os algoritmos avaliados num cenário. `process_count` e a seed continuam livres para cada execução/seed do experimento.

| Cenário | `ScenarioKind` | Rajada de CPU | Nº de E/S | Prioridade |
|---|---|---|---|---|
| Aleatório equilibrado | `SCENARIO_BALANCED` | 1-20 (curtas e longas) | 0-5 (pouca e muita E/S) | uniforme 1-5 |
| I/O-bound | `SCENARIO_IO_BOUND` | 1-4 (curtas) | 3-8 (muita E/S) | uniforme 1-5 |
| CPU-bound / processos longos | `SCENARIO_CPU_BOUND` | 15-40 (longas) | 0-1 (quase nenhuma) | uniforme 1-5 |
| Prioridades desbalanceadas | `SCENARIO_UNBALANCED_PRIORITY` | 1-20 (igual ao equilibrado) | 0-5 (igual ao equilibrado) | 80% em 1-2 (alta), 20% em 3-5 (baixa) |

- Os cenários 1-3 variam apenas a forma da carga (CPU/E-S); a prioridade é
  uniforme nos três, para isolar o efeito de cada forma nas métricas.
- O cenário de prioridades desbalanceadas reusa a mesma forma de carga do
  equilibrado - o único eixo alterado é a distribuição de prioridade - para
  isolar o efeito do desbalanceamento em si.
- O desbalanceamento de prioridade é implementado como uma mistura: com
  probabilidade `high_priority_fraction`, a prioridade é sorteada na fatia
  alta (`[min_priority, high_priority_boundary]`); caso contrário, na fatia
  baixa. Isso está em `WorkloadParams`/`workload_generate`, disponível para
  qualquer cenário que precise de prioridade não uniforme.
- Testado em `tests/test_scenarios.c`: os 4 cenários geram cargas válidas,
  reprodutibilidade por seed, I/O-bound tem mais E/S e menos CPU que
  CPU-bound (garantido pelas faixas disjuntas, não é um teste estatístico),
  e o cenário desbalanceado favorece a alta prioridade.

## Modelagem de E/S

- **Quando um processo solicita E/S:** ao esgotar uma rajada de CPU que não é
  a última da sua sequência (`bursts`), imediatamente na transição
  `RUNNING -> BLOCKED`.
- **Por quanto tempo permanece bloqueado:** pela duração da rajada de E/S
  seguinte em `spec.bursts` (os índices ímpares do vetor).
- **Paralelismo:** múltiplas E/S ocorrem em paralelo sem limite; cada
  processo bloqueia de forma independente, sem fila ou contenção com os
  demais.
- **Dispositivo(s):** não há uma estrutura de dispositivo físico modelada -
  isso equivale a um dispositivo dedicado (sem fila própria) por processo,
  já que nenhuma E/S espera pela conclusão de outra.
- **Retorno à fila de prontos:** uma E/S iniciada em `t` com duração `d`
  agenda `unblock_time = t + d`; quando o relógio da simulação alcança esse
  instante, o processo volta a `READY` (`BLOCKED -> READY`).

Essa escolha simplifica o modelo (sem contenção por dispositivo) e é a mesma
para todos os algoritmos avaliados; a limitação é não capturar cenários onde um dispositivo real seria gargalo (ex.: disco único disputado por muitos processos).

## Transições de estado entre CPU e E/S

```
NEW ---(chegada)--> READY ---(escolhido pelo escalonador)--> RUNNING
                       ^                                        |
                       |                                        |--(rajada de CPU esgota, ha proxima rajada)--> BLOCKED
                       |                                        |--(rajada de CPU esgota, era a ultima)-------> FINISHED
                       |                                        |--(preempcao, ex.: fim do quantum no RR)-----> READY
                       |                                        |
                       +----------(E/S conclui: unblock_time)---BLOCKED
```

- `READY -> RUNNING`: o escalonador escolhe o processo (`choose`).
- `RUNNING -> BLOCKED`: fim de uma rajada de CPU seguida de rajada de E/S.
- `BLOCKED -> READY`: fim da rajada de E/S (`unblock_time` alcançado).
- `RUNNING -> READY`: preempção (apenas no Round Robin, ao esgotar o
  quantum).
- `RUNNING -> FINISHED`: fim da última rajada de CPU do processo.

## Troca de contexto

- **Quando ocorre:** apenas quando a CPU passa diretamente de um processo
  para outro com PID diferente. A primeira execução (CPU estava ociosa) e a
  retomada do mesmo processo (ex.: único pronto após esgotar o quantum) não
  contam como troca.
- **Duração:** `config.context_switch_cost` unidades de tempo, configurável
  e obrigatoriamente maior que zero nos experimentos principais (análises
  complementares podem usar custo zero). O mesmo valor é usado para todos os
  algoritmos comparados num mesmo experimento.
- **CPU durante a troca:** fica indisponível - nenhum processo executa
  durante o custo da troca (a linha do tempo registra `SIM_CONTEXT_SWITCH`
  em cada unidade). O relógio continua avançando e chegadas/retornos de E/S
  são processados normalmente nesse intervalo.
- **Contabilização:** a troca é contada quando a CPU sai de um PID e entra
  em outro PID; sair do estado ocioso para executar um processo **não** é
  contabilizado como troca.

## Convenções do modelo

- O tempo é discreto e cada unidade executa uma unidade de rajada de CPU.
- Valores numéricos menores representam prioridades maiores.
- E/S de processos diferentes ocorre em paralelo, sem fila de dispositivo. Uma
  E/S iniciada em `t` com duração `d` devolve o processo à fila em `t + d`.
- Chegadas e conclusões de E/S são processadas antes da decisão de escalonamento;
  se coincidirem, chegadas entram primeiro, ambas em ordem do vetor da carga.
- FCFS e Round Robin escolhem pela ordem de entrada na fila de prontos.
- Em prioridade, o desempate é: entrada na fila, chegada e ID.
- FCFS e prioridade são não preemptivos durante uma rajada de CPU. Bloqueio ou
  término liberam a CPU.
- No Round Robin, o processo que esgota o quantum volta ao fim da fila. Se ele
  for o único pronto, é selecionado novamente sem troca de processo.
- A escolha do próximo processo ocorre antes do custo da troca; eventos que aconteçam durante esse custo entram na fila para a decisão seguinte.
- A linha do tempo usa `SIM_IDLE` e `SIM_CONTEXT_SWITCH` para representar CPU
  ociosa e custo de troca. Ela pode ser desativada em cargas grandes.

O resultado expõe chegada, conclusão, CPU e E/S totais por processo, além do
número de trocas. Isso permite calcular turnaround, slowdown e índice de Jain
sem duplicar a lógica do simulador.

## EPA - Escalonador Preditivo Adaptativo (algoritmo próprio da equipe)

Implementado em `src/scheduler_epa.c` (documentação detalhada nos comentários
do arquivo: problema que resolve, dados que usa, formula de decisão e
limitações conhecidas). Resumo:

- Não preemptivo por rajada, como FCFS e prioridade: só decide quando a CPU
  fica livre.
- Prevê a próxima rajada de CPU de cada processo por média móvel exponencial
  sobre as rajadas de CPU **já concluídas** desse processo (nunca espia
  `spec.bursts` além do que já aconteceu).
- Usa envelhecimento por tempo real de espera para garantir espera limitada
  (sem inanição indefinida), diferente da prioridade estática clássica.
- Dá um bônus leve a processos historicamente mais ligados a E/S, para manter
  dispositivos ocupados e reduzir o slowdown de processos interativos.
- A prioridade estática da carga entra apenas como desempate suave.

Os pesos atuais do EPA (`EPA_AGING_WEIGHT`, `EPA_IO_WEIGHT` e
`EPA_PRIORITY_WEIGHT`) são constantes de compilação. O algoritmo possui teste
determinístico e participa do executor dos quatro cenários obrigatórios.

## Experimentos, métricas e gráficos

Compile o executor experimental:

```sh
make experiment
```

Execute a configuração mínima oficial (1.000 processos, 100 seeds, quantum 4
e custo de troca 1):

```sh
./experiment dados/processos.csv dados/execucoes.csv 1000 100 4 1
```

O executor cria os diretórios de saída e registra em `execucoes.csv` o número
de processos, o total de seeds, o quantum e o custo da troca. Para calcular as
métricas, IC95% e gráficos:

```sh
python analysis/analisar.py --processos dados/processos.csv --execucoes dados/execucoes.csv --saida resultados
python analysis/comparar_resultados.py --resumo resultados/resumo_ic95.csv
```

`make test` executa os testes em C e os testes Python das métricas e da análise
comparativa. A configuração usada nos resultados versionados também está em
`resultados/configuracao_experimental.csv`.
