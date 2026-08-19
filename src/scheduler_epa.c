#include "scheduler_internal.h"

/*
 * EPA - Escalonador Preditivo Adaptativo
 * =======================================
 * Algoritmo proprio da equipe (parte do Abner). Nao preemptivo por rajada:
 * assim como FCFS e prioridade, so decide quando a CPU fica livre (termino
 * de rajada, bloqueio ou chegada em CPU ociosa); nao interrompe um processo
 * em execucao. Isso o mantem compativel com o nucleo atual (simulator.c) sem
 * exigir a logica de preempcao por quantum que hoje e exclusiva do Round
 * Robin.
 *
 * (a) Problema que tenta resolver
 * -------------------------------
 * Os tres algoritmos classicos do projeto tem cada um uma fraqueza conhecida:
 *   - FCFS sofre o "efeito comboio": um processo CPU-bound longo na frente
 *     atrasa varios processos curtos atras dele.
 *   - Prioridade estatica nao-preemptiva pode causar inanicao (starvation)
 *     indefinida de processos de prioridade baixa, e ignora completamente o
 *     comportamento observado do processo (se ele tende a rajadas curtas ou
 *     longas, CPU-bound ou I/O-bound).
 *   - Round Robin e justo no tempo de CPU, mas trata todo processo igual,
 *     nao aproveitando processos I/O-bound para manter dispositivos de E/S
 *     ocupados enquanto a CPU atende outro processo.
 * O EPA tenta reduzir o tempo medio de espera/turnaround (como um SJF
 * preditivo) e o slowdown de processos interativos (dando leve preferencia a
 * quem tem alta afinidade de E/S), enquanto GARANTE espera limitada (sem
 * inanicao indefinida) atraves de envelhecimento por tempo real decorrido.
 *
 * (b) Quais informacoes usa (e quais NAO usa)
 * --------------------------------------------
 * Usa exclusivamente dados que o sistema operacional teria disponiveis em
 * tempo real, ate o instante da decisao:
 *   - epa_predicted_burst: estimativa da PROXIMA rajada de CPU do processo,
 *     calculada por media movel exponencial (EWMA) sobre as rajadas de CPU
 *     JA CONCLUIDAS desse mesmo processo (formula classica de estimativa de
 *     proxima rajada de CPU, Silberschatz et al.). Antes de qualquer rajada
 *     observada, usa uma constante neutra (EPA_INITIAL_BURST_ESTIMATE), igual
 *     para todos os processos - nunca espia a duracao real da primeira
 *     rajada em spec.bursts[0], pois isso seria conhecimento futuro.
 *   - epa_waiting_ticks: unidades de tempo decorridas desde que o processo
 *     entrou (ou reentrou) na fila de prontos - tempo real, nao apenas
 *     posicao/ordem na fila.
 *   - epa_observed_cpu_time / epa_observed_io_time: soma das rajadas de CPU e
 *     de E/S ja CONCLUIDAS (ou ja iniciadas, no caso da E/S corrente), usada
 *     para estimar o "perfil" do processo (mais CPU-bound ou mais I/O-bound).
 *   - spec.priority: entrada estatica fornecida pela carga de trabalho (nao
 *     e "conhecimento futuro" - e um parametro de entrada, exatamente como
 *     ja e usado pelo escalonador de prioridade classico), usada apenas como
 *     desempate suave.
 *   - ready_order / spec.id: desempates finais, para determinismo.
 *
 * O algoritmo NUNCA le spec.bursts em indices alem de burst_index (o que ja
 * aconteceu), nem usa RuntimeProcess::remaining como se fosse a duracao total
 * conhecida da rajada atual (isso seria SJF com conhecimento perfeito, que e
 * irrealista - por isso a rajada e estimada, nunca lida diretamente).
 *
 * (c) Como escolhe o proximo processo
 * ------------------------------------
 * Entre os processos em estado PROCESS_READY, calcula uma pontuacao (menor
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
 *   - predicted_burst baixo favorece processos com historico de rajadas
 *     curtas (como SJF, reduz turnaround medio).
 *   - waiting_ticks alto reduz a pontuacao sem limite: por mais que um
 *     processo tenha rajadas previstas longas, apos esperar tempo suficiente
 *     seu score cai abaixo de qualquer concorrente, garantindo que ele sera
 *     escolhido eventualmente (espera limitada, sem inanicao indefinida).
 *   - io_affinity alta reduz levemente a pontuacao de processos historicamente
 *     I/O-bound, priorizando-os para que voltem rapido a fila de E/S e essa
 *     fique mais ocupada (menor tempo ocioso de dispositivos, menor slowdown
 *     percebido por processos interativos).
 *   - spec.priority funciona como desempate suave (nao domina a formula),
 *     preservando alguma influencia da prioridade estatica sem reintroduzir
 *     o risco de inanicao que ela causa isoladamente.
 * Em empate exato de score, desempata por ready_order e depois por id, como
 * os demais escalonadores do projeto.
 *
 * (d) Por que deve melhorar a simulacao, e limitacoes
 * -----------------------------------------------------
 * Esperado, frente aos classicos:
 *   - Turnaround medio menor que FCFS em cargas heterogeneas (evita o efeito
 *     comboio) sem o custo de troca de contexto do RR a cada quantum.
 *   - Menor slowdown de processos curtos/interativos que a prioridade
 *     estatica, sem os riscos de inanicao dela.
 *   - Ao contrario da prioridade estatica, se adapta durante a execucao: um
 *     processo que passa a ter rajadas mais curtas (ou mais E/S) melhora sua
 *     posicao na fila automaticamente.
 *
 * Limitacoes conhecidas (documentar no artigo/analise):
 *   - E uma heuristica com pesos fixos (constantes abaixo); nao ha, no
 *     momento, um mecanismo para ajustar esses pesos por SimulationConfig
 *     (ver bloqueio de integracao no topo de simulator.c / README da tarefa).
 *   - A previsao EWMA erra sistematicamente para processos cujo padrao de
 *     rajadas muda de forma abrupta (nao gradual) - e uma limitacao conhecida
 *     e aceita de qualquer previsor baseado em historico, nao exclusiva
 *     deste algoritmo.
 *   - epa_waiting_ticks conta o tick em que o processo acabou de ficar pronto
 *     como 1 unidade de espera (ver comentario em epa_tick_waiting, em
 *     simulator.c); e uma aproximacao de no maximo 1 tick, sem impacto na
 *     garantia de espera limitada.
 *   - Nao ha, ainda, um cenario dos 4 obrigatorios (parte da Maria) rodando
 *     este algoritmo em teste de integracao - ver TODO em tests/test_schedulers.c.
 */

#define EPA_AGING_WEIGHT 1.0
#define EPA_IO_WEIGHT 3.0
#define EPA_PRIORITY_WEIGHT 0.5

int scheduler_choose_epa(const RuntimeProcess *processes, size_t count) {
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
