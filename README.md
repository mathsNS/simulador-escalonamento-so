# Simulador de escalonamento de processos

Simulador discreto em C dos algoritmos clássicos FCFS, Round Robin e prioridade
não preemptiva. Os processos são descritos por rajadas alternadas
`CPU, E/S, CPU, ..., CPU`.

## Compilação e execução

```sh
make
./simulator fcfs
./simulator rr 2 1
./simulator priority 2 1
make test
```

Os argumentos da demonstração são algoritmo, quantum e custo da troca de
contexto. O quantum só é usado pelo Round Robin. Para integrar outra carga,
use a API pública em `include/simulator.h`.

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
- Uma troca é contabilizada quando a CPU passa diretamente entre PIDs distintos.
  A primeira execução e a retomada após período ocioso não contam. Durante seu
  custo configurável a CPU não executa processos, mas chegadas e E/S avançam.
- A escolha do próximo processo ocorre antes do custo da troca; eventos que
  aconteçam durante esse custo entram na fila para a decisão seguinte.
- A linha do tempo usa `SIM_IDLE` e `SIM_CONTEXT_SWITCH` para representar CPU
  ociosa e custo de troca. Ela pode ser desativada em cargas grandes.

O resultado expõe chegada, conclusão, CPU e E/S totais por processo, além do
número de trocas. Isso permite calcular turnaround, slowdown e índice de Jain
sem duplicar a lógica do simulador.
