# Roteiro da seção de métricas, experimentos e resultados

## Metodologia experimental

O experimento deve ser executado nos quatro cenários obrigatórios: equilibrado, I/O-bound,
CPU-bound e prioridades desbalanceadas. Para cada cenário, devem ser usadas as mesmas cargas e as
mesmas seeds nos quatro algoritmos. A configuração mínima é de 1.000 processos por execução e 100
seeds por cenário. Também devem ser registrados o quantum do Round Robin e o custo de troca de
contexto, mantido igual entre os algoritmos.

Cada seed é uma repetição independente e, portanto, é a unidade amostral usada no IC95%. Primeiro as
métricas são calculadas dentro de cada execução. Depois, os valores das diferentes seeds são
agregados. Os milhares de processos de uma mesma seed não devem ser tratados como repetições
independentes no cálculo do intervalo de confiança.

## Definições

Para o processo `i`:

- `turnaround_i = termino_i - chegada_i`;
- `tempo_ideal_i = soma_cpu_i + soma_io_i`;
- `slowdown_i = turnaround_i / tempo_ideal_i`.

Para uma execução com `n` processos:

- turnaround médio: média dos turnarounds dos processos;
- trocas de contexto: total registrado pelo simulador naquela execução;
- Jain do slowdown: `(soma(slowdown)^2 / (n * soma(slowdown^2))) * 100`.

Para cada combinação de cenário e algoritmo, a média e o intervalo entre seeds são:

- `IC95% = media +/- 1,96 * desvio_padrao_amostral / sqrt(numero_de_seeds)`.

## Checklist antes dos experimentos oficiais

- Definir precisamente quando uma troca de contexto é contabilizada.
- Confirmar se a saída do estado ocioso conta como troca e manter a regra em todos os algoritmos.
- Confirmar que o custo da troca é maior que zero e igual entre algoritmos.
- Fixar e registrar o quantum do Round Robin.
- Salvar configuração, cenário, algoritmo e seed em cada resultado.
- Verificar que cada algoritmo recebeu processos idênticos para a mesma seed/cenário.
- Fazer uma execução pequena e conferir manualmente as métricas.
- Executar ao menos 100 seeds e 1.000 processos por seed nos resultados oficiais.

## Perguntas para a discussão

Para cada cenário, comparar o algoritmo próprio com FCFS, Round Robin e Prioridade:

1. Qual algoritmo obteve o menor turnaround médio? Os IC95% se sobrepõem?
2. Qual gerou mais trocas de contexto e por qual decisão de escalonamento?
3. Qual obteve o maior índice de Jain? Esse ganho de justiça veio acompanhado de pior turnaround?
4. O algoritmo próprio favoreceu cargas I/O-bound, CPU-bound ou prioridades desbalanceadas?
5. Em qual cenário o algoritmo próprio foi pior e qual mecanismo explica o resultado?
6. O custo introduzido pelo algoritmo próprio compensa os benefícios observados?

Evitar afirmar apenas que um algoritmo é “melhor”. Toda conclusão deve indicar cenário, métrica,
comparador e possível causa. Sobreposição de IC95% não é, sozinha, um teste formal de igualdade ou de
significância estatística.

## Estrutura sugerida para apresentar os resultados

1. Tabela com a configuração experimental completa.
2. Gráfico de turnaround médio com IC95%.
3. Gráfico de trocas de contexto com IC95%.
4. Gráfico do índice de Jain do slowdown com IC95%.
5. Discussão por cenário, concentrada no algoritmo próprio.
6. Síntese dos ganhos, custos e limitações.