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

### Pontos de integração ainda em aberto (dependências de equipe)

Estes pontos exigem coordenação com quem mantém o núcleo (Maria) e a
interface comum dos algoritmos (Matheus) antes de serem finalizados:

1. **Pesos configuráveis pelo usuário.** Hoje os pesos do EPA
   (`EPA_AGING_WEIGHT`, `EPA_IO_WEIGHT`, `EPA_PRIORITY_WEIGHT` em
   `scheduler_epa.c`) são constantes de compilação, porque `ChooseProcess`
   tem assinatura `(const RuntimeProcess *, size_t)` e não recebe
   `SimulationConfig`. Tornar isso configurável exigiria estender essa
   assinatura (mudança de baixo risco, mas que toca os 4 arquivos de
   escalonador) - ver TODO em `scheduler_internal.h`.
2. **Validação nos 4 cenários obrigatórios.** Ainda não há um teste de
   integração do EPA num dos cenários formais do projeto (parte da Maria);
   os testes atuais em `tests/test_schedulers.c` cobrem apenas casos
   isolados e hand-verificados.
3. **Comparação quantitativa.** Turnaround médio, slowdown e índice de Jain
   do EPA frente aos clássicos dependem do harness de métricas/experimentos
   (parte compartilhada da equipe) - o simulador já expõe os dados brutos
   necessários (ver seção anterior), falta só o script de análise.

Enquanto isso, o EPA já compila, roda (`./simulator epa`) e tem cobertura de
teste básica (`make test`), podendo ser usado desde já nos 3 pontos acima
assim que as partes correspondentes estiverem prontas.
