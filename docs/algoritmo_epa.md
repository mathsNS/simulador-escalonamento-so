# EPA — Escalonador Preditivo Adaptativo

O EPA é a política de escalonamento proposta para este projeto. Seu objetivo é
combinar três propriedades em uma única decisão:

- favorecer processos com histórico de rajadas curtas;
- aumentar gradualmente a preferência por processos que aguardam na fila;
- considerar o comportamento de E/S já observado.

O algoritmo é não preemptivo por rajada. Depois de receber a CPU, um processo
executa até concluir a rajada atual ou iniciar uma operação de E/S. Uma nova
escolha ocorre quando a CPU fica livre.

## Informações utilizadas

Para cada processo, o simulador mantém:

- `epa_predicted_burst`: estimativa da próxima rajada de CPU;
- `epa_waiting_ticks`: tempo desde a entrada mais recente na fila de prontos;
- `epa_observed_cpu_time`: CPU consumida em rajadas já concluídas;
- `epa_observed_io_time`: duração das operações de E/S já iniciadas;
- prioridade estática, ordem de entrada na fila e PID.

O EPA não consulta a duração restante da rajada atual nem rajadas futuras. A
primeira estimativa é igual para todos os processos:

```text
estimativa_inicial = 5
```

Assim, processos sem histórico são diferenciados pelo tempo de espera, pela
prioridade e pelos critérios de desempate.

## Previsão da próxima rajada

Ao final de uma rajada de CPU, a previsão é atualizada por média móvel
exponencial, com peso `0,5` para a observação mais recente:

```text
nova_previsao = 0,5 * rajada_observada + 0,5 * previsao_anterior
```

Rajadas recentes influenciam a estimativa, mas o histórico anterior não é
descartado completamente. Esse mecanismo permite adaptar a escolha ao
comportamento observado sem pressupor conhecimento antecipado da carga.

## Afinidade com E/S

A afinidade é calculada a partir dos tempos acumulados já observados:

```text
afinidade_es = es_observada / (cpu_observada + es_observada)
```

Antes de existir histórico, a afinidade vale zero. Valores próximos de `1`
indicam maior proporção de E/S; valores próximos de `0`, maior proporção de
CPU. O termo funciona como um bônus moderado para processos com maior afinidade
com E/S.

## Envelhecimento

`epa_waiting_ticks` é incrementado a cada unidade de tempo em que o processo
permanece pronto. O contador é reiniciado quando ele entra ou retorna à fila.
Como o tempo de espera reduz continuamente a pontuação, um processo adquire
preferência enquanto aguarda, reduzindo o risco de inanição.

O envelhecimento não torna o algoritmo preemptivo: uma rajada em execução não
é interrompida, mesmo que outro processo acumule uma pontuação melhor.

## Regra de escolha

Para cada processo pronto, o EPA calcula:

```text
score = previsao_rajada
      - 1,0 * tempo_espera
      - 3,0 * afinidade_es
      + 0,5 * prioridade
```

O processo com menor `score` é escolhido. Como valores numéricos menores
representam prioridades maiores, o último termo preserva a convenção adotada
pelo simulador.

Os pesos atuais são constantes de compilação em `src/scheduler_epa.c`:

| Termo | Constante | Valor |
|---|---|---:|
| Envelhecimento | `EPA_AGING_WEIGHT` | 1,0 |
| Afinidade com E/S | `EPA_IO_WEIGHT` | 3,0 |
| Prioridade estática | `EPA_PRIORITY_WEIGHT` | 0,5 |

## Critérios de desempate

Se dois processos obtiverem exatamente o mesmo `score`, a escolha segue:

1. menor `ready_order`, isto é, quem entrou primeiro na fila de prontos;
2. menor PID.

Esses critérios tornam a execução determinística para uma mesma carga e seed.

## Exemplo

Considere dois processos prontos:

| Processo | Previsão | Espera | Afinidade E/S | Prioridade |
|---|---:|---:|---:|---:|
| P1 | 6 | 2 | 0,25 | 1 |
| P2 | 4 | 0 | 0,50 | 3 |

As pontuações são:

```text
P1 = 6 - 2 - 3*0,25 + 0,5*1 = 3,75
P2 = 4 - 0 - 3*0,50 + 0,5*3 = 4,00
```

O EPA escolhe P1. Embora sua rajada prevista seja maior, o tempo de espera e a
prioridade compensam essa diferença.

## Comparação com os algoritmos clássicos

| Política | Diferença principal em relação ao EPA |
|---|---|
| FCFS | Usa somente a ordem de entrada na fila. |
| Round Robin | É preemptivo e divide a CPU por quantum. |
| Prioridade | Usa prioridade estática como critério principal. |
| EPA | Combina histórico, espera, E/S e prioridade em uma pontuação dinâmica. |

O EPA compartilha com FCFS e Prioridade o comportamento não preemptivo por
rajada. Diferentemente deles, sua ordem pode mudar conforme o histórico de
execução. Em relação ao Round Robin, evita preempções periódicas, mas não oferece
a mesma alternância imediata entre processos prontos.

## Complexidade

A fila de prontos é representada pelo conjunto de PCBs no estado `READY`. Cada
decisão percorre os `n` processos uma vez, portanto a escolha tem custo
`O(n)` e usa memória auxiliar `O(1)`. O histórico persistente ocupa quatro
campos por processo, totalizando `O(n)`.

## Limitações

- Os pesos são fixos e foram mantidos iguais em todos os cenários.
- A estimativa inicial única pode aproximar o comportamento do FCFS antes de
  existir histórico suficiente.
- A média móvel reage com atraso a mudanças bruscas no padrão de rajadas.
- O bônus de E/S depende apenas do histórico acumulado e não distingue tipos de
  dispositivo.
- Por ser não preemptivo, um processo não pode interromper uma rajada longa que
  já esteja em execução.
- O envelhecimento reduz o risco de inanição na fila, mas não limita a duração
  da rajada que ocupa a CPU.

## Implementação e avaliação

A escolha está implementada em `src/scheduler_epa.c`. A atualização do histórico
ocorre em `src/simulator.c`, somente depois que cada informação se torna
observável. O teste determinístico está em `tests/test_schedulers.c`.

O executor `src/experiment.c` avalia o EPA com as mesmas cargas usadas pelos
algoritmos clássicos. Os resultados consolidados e os intervalos de confiança
estão em `resultados/`, e a metodologia estatística está descrita em
`docs/metodologia_experimental.md`.
