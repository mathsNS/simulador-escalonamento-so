#include "scheduler_internal.h"

/*
 * EPA - Escalonador Preditivo Adaptativo
 * =======================================
 * Algoritmo próprio da equipe (parte do Abner). Não preemptivo por rajada:
 * assim como FCFS e prioridade, só decide quando a CPU fica livre (término
 * de rajada, bloqueio ou chegada em CPU ociosa); não interrompe um processo
 * em execução. Isso o mantém compatível com o núcleo atual (simulator.c) sem
 * exigir a lógica de preempção por quantum que hoje é exclusiva do Round
 * Robin.
 *
 * (a) Problema que tenta resolver
 * -------------------------------
 * Os três algoritmos clássicos do projeto têm, cada um, uma fraqueza conhecida:
 *   - FCFS sofre o "efeito comboio": um processo CPU-bound longo na frente
 *     atrasa vários processos curtos atrás dele.
 *   - Prioridade estática não preemptiva pode causar inanição (starvation)
 *     indefinida de processos de prioridade baixa, e ignora completamente o
 *     comportamento observado do processo (se ele tende a rajadas curtas ou
 *     longas, CPU-bound ou I/O-bound).
 *   - Round Robin é justo no tempo de CPU, mas trata todo processo igual,
 *     não aproveitando processos I/O-bound para manter dispositivos de E/S
 *     ocupados enquanto a CPU atende outro processo.
 * O EPA tenta reduzir o tempo médio de espera/turnaround (como um SJF
 * preditivo) e o slowdown de processos interativos (dando leve preferência a
 * quem tem alta afinidade de E/S), enquanto GARANTE espera limitada (sem
 * inanição indefinida) através de envelhecimento por tempo real decorrido.
 *
 * (b) Quais informações usa (e quais NÃO usa)
 * --------------------------------------------
 * Usa exclusivamente dados que o sistema operacional teria disponíveis em
 * tempo real, até o instante da decisão:
 *   - epa_predicted_burst: estimativa da PRÓXIMA rajada de CPU do processo,
 *     calculada por média móvel exponencial (EWMA) sobre as rajadas de CPU
 *     JÁ CONCLUÍDAS desse mesmo processo (fórmula clássica de estimativa de
 *     próxima rajada de CPU, Silberschatz et al.). Antes de qualquer rajada
 *     observada, usa uma constante neutra (EPA_INITIAL_BURST_ESTIMATE), igual
 *     para todos os processos - nunca espia a duração real da primeira
 *     rajada em spec.bursts[0], pois isso seria conhecimento futuro.
 *   - epa_waiting_ticks: unidades de tempo decorridas desde que o processo
 *     entrou (ou reentrou) na fila de prontos - tempo real, não apenas
 *     posição/ordem na fila.
 *   - epa_observed_cpu_time / epa_observed_io_time: soma das rajadas de CPU e
 *     de E/S já CONCLUÍDAS (ou já iniciadas, no caso da E/S corrente), usada
 *     para estimar o "perfil" do processo (mais CPU-bound ou mais I/O-bound).
 *   - spec.priority: entrada estática fornecida pela carga de trabalho (não
 *     é "conhecimento futuro" - é um parâmetro de entrada, exatamente como
 *     já é usado pelo escalonador de prioridade clássico), usada apenas como
 *     desempate suave.
 *   - ready_order / spec.id: desempates finais, para determinismo.
 *
 * O algoritmo NUNCA lê spec.bursts em índices além de burst_index (o que já
 * aconteceu), nem usa PCB::remaining como se fosse a duração total
 * conhecida da rajada atual (isso seria SJF com conhecimento perfeito, que é
 * irrealista - por isso a rajada é estimada, nunca lida diretamente).
 *
 * (c) Como escolhe o próximo processo
 * ------------------------------------
 * Entre os processos em estado PROCESS_READY, calcula uma pontuação (menor
 * vence):
 *
 *   score = predicted_burst
 *         - AGING_WEIGHT      * waiting_ticks
 *         - IO_WEIGHT         * io_affinity        (io_affinity em [0, 1])
 *         + PRIORITY_WEIGHT   * spec.priority
 *
 * onde io_affinity = observed_io_time / (observed_io_time + observed_cpu_time).
 *
 * Efeito de cada termo:
 *   - predicted_burst baixo favorece processos com histórico de rajadas
 *     curtas (como SJF, reduz turnaround médio).
 *   - waiting_ticks alto reduz a pontuação sem limite: por mais que um
 *     processo tenha rajadas previstas longas, após esperar tempo suficiente
 *     seu score cai abaixo de qualquer concorrente, garantindo que ele será
 *     escolhido eventualmente (espera limitada, sem inanição indefinida).
 *   - io_affinity alta reduz levemente a pontuação de processos historicamente
 *     I/O-bound, priorizando-os para que voltem rápido à fila de E/S e essa
 *     fique mais ocupada (menor tempo ocioso de dispositivos, menor slowdown
 *     percebido por processos interativos).
 *   - spec.priority funciona como desempate suave (não domina a fórmula),
 *     preservando alguma influência da prioridade estática sem reintroduzir
 *     o risco de inanição que ela causa isoladamente.
 * Em empate exato de score, desempata por ready_order e depois por id, como
 * os demais escalonadores do projeto.
 *
 * (d) Por que deve melhorar a simulação, e limitações
 * -----------------------------------------------------
 * Esperado, frente aos clássicos:
 *   - Turnaround médio menor que FCFS em cargas heterogêneas (evita o efeito
 *     comboio) sem o custo de troca de contexto do RR a cada quantum.
 *   - Menor slowdown de processos curtos/interativos que a prioridade
 *     estática, sem os riscos de inanição dela.
 *   - Ao contrário da prioridade estática, se adapta durante a execução: um
 *     processo que passa a ter rajadas mais curtas (ou mais E/S) melhora sua
 *     posição na fila automaticamente.
 *
 * Limitações conhecidas (documentar no artigo/análise):
 *   - É uma heurística com pesos fixos (constantes abaixo); não há, no
 *     momento, um mecanismo para ajustar esses pesos por SimulationConfig
 *     (ver bloqueio de integração no topo de simulator.c / README da tarefa).
 *   - A previsão EWMA erra sistematicamente para processos cujo padrão de
 *     rajadas muda de forma abrupta (não gradual) - é uma limitação conhecida
 *     e aceita de qualquer previsor baseado em histórico, não exclusiva
 *     deste algoritmo.
 *   - epa_waiting_ticks conta o tick em que o processo acabou de ficar pronto
 *     como 1 unidade de espera (ver comentário em epa_tick_waiting, em
 *     simulator.c); é uma aproximação de no máximo 1 tick, sem impacto na
 *     garantia de espera limitada.
 *   - Não há, ainda, um cenário dos 4 obrigatórios (parte da Maria) rodando
 *     este algoritmo em teste de integração - ver TODO em tests/test_schedulers.c.
 */

#define EPA_AGING_WEIGHT 1.0
#define EPA_IO_WEIGHT 3.0
#define EPA_PRIORITY_WEIGHT 0.5

int scheduler_choose_epa(const PCB *processes, size_t count) {
    int best = -1;
    double best_score = 0.0;
    size_t i;

    for (i = 0; i < count; ++i) {
        double cpu_total, io_total, io_affinity, score;

        if (processes[i].state != PROCESS_READY)
            continue;

        cpu_total = (double)processes[i].epa_observed_cpu_time;
        io_total = (double)processes[i].epa_observed_io_time;
        io_affinity = (cpu_total + io_total > 0.0) ? io_total / (cpu_total + io_total) : 0.0;

        score = processes[i].epa_predicted_burst
              - EPA_AGING_WEIGHT * (double)processes[i].epa_waiting_ticks
              - EPA_IO_WEIGHT * io_affinity
              + EPA_PRIORITY_WEIGHT * (double)processes[i].spec.priority;

        if (best < 0 || score < best_score ||
            (score == best_score &&
             (processes[i].ready_order < processes[best].ready_order ||
              (processes[i].ready_order == processes[best].ready_order &&
               processes[i].spec.id < processes[best].spec.id)))) {
            best = (int)i;
            best_score = score;
        }
    }
    return best;
}
