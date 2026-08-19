# Análise comparativa dos algoritmos

Os valores abaixo são médias entre seeds. Os intervalos apresentados são IC95% calculados com as seeds como repetições independentes.

> A sobreposição de IC95% é apenas um auxílio visual e não constitui, isoladamente, um teste formal de significância estatística.

## Cenário: equilibrado

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 18600.48 | [18500.85; 18700.11] |
| fcfs | 22730.91 | [22580.95; 22880.87] |
| epa | 22731.50 | [22581.53; 22881.48] |
| round_robin | 28568.69 | [28388.04; 28749.34] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 22.21%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 3495.68 | [3483.98; 3507.38] |
| prioridade | 3495.68 | [3483.98; 3507.38] |
| epa | 3495.68 | [3483.98; 3507.38] |
| round_robin | 10483.44 | [10442.78; 10524.10] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa / fcfs / prioridade**. Em relação ao melhor clássico (**fcfs / prioridade**), o EPA apresentou resultado igual. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 531.09 | [527.31; 534.86] |
| fcfs | 531.14 | [527.36; 534.92] |
| round_robin | 602.22 | [600.47; 603.98] |
| prioridade | 760.88 | [747.73; 774.04] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 0.01%. Os IC95% do EPA e desse clássico se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 79.64 | [79.04; 80.25] |
| epa | 41.01 | [39.68; 42.34] |
| fcfs | 41.01 | [39.67; 42.34] |
| prioridade | 12.94 | [12.37; 13.52] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 48.51%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Cenário: io_bound

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 11104.77 | [11071.63; 11137.91] |
| epa | 16146.01 | [16095.54; 16196.48] |
| fcfs | 16148.74 | [16098.26; 16199.23] |
| round_robin | 16148.74 | [16098.26; 16199.23] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 45.40%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 6495.16 | [6483.46; 6506.86] |
| round_robin | 6495.16 | [6483.46; 6506.86] |
| prioridade | 6495.16 | [6483.46; 6506.86] |
| epa | 6495.16 | [6483.46; 6506.86] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa / fcfs / prioridade / round_robin**. Em relação ao melhor clássico (**fcfs / prioridade / round_robin**), o EPA apresentou resultado igual. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 225.36 | [224.69; 226.03] |
| epa | 310.72 | [310.08; 311.36] |
| fcfs | 310.77 | [310.13; 311.41] |
| round_robin | 310.77 | [310.13; 311.41] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 37.88%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 92.81 | [92.73; 92.89] |
| round_robin | 92.81 | [92.73; 92.89] |
| epa | 92.79 | [92.71; 92.87] |
| prioridade | 65.35 | [65.16; 65.53] |

O resultado favorável é o maior. O melhor resultado geral foi de **fcfs / round_robin**. Em relação ao melhor clássico (**fcfs / round_robin**), o EPA apresentou desvantagem de 0.02%. Os IC95% do EPA e desse clássico se sobrepõem.

## Cenário: cpu_bound

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 17935.29 | [17857.33; 18013.26] |
| fcfs | 20001.14 | [19901.44; 20100.84] |
| epa | 20002.77 | [19903.05; 20102.50] |
| round_robin | 34231.81 | [34095.81; 34367.82] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 11.53%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 1497.89 | [1494.64; 1501.14] |
| prioridade | 1497.89 | [1494.64; 1501.14] |
| epa | 1497.89 | [1494.64; 1501.14] |
| round_robin | 10829.24 | [10798.52; 10859.96] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa / fcfs / prioridade**. Em relação ao melhor clássico (**fcfs / prioridade**), o EPA apresentou resultado igual. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 487.71 | [486.03; 489.39] |
| fcfs | 487.72 | [486.04; 489.40] |
| prioridade | 495.74 | [493.61; 497.87] |
| round_robin | 845.84 | [844.18; 847.50] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 0.00%. Os IC95% do EPA e desse clássico se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 94.29 | [94.23; 94.35] |
| epa | 77.74 | [77.60; 77.88] |
| fcfs | 77.73 | [77.59; 77.87] |
| prioridade | 57.04 | [56.85; 57.24] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 17.55%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Cenário: prioridades_desbalanceadas

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 19283.28 | [19182.59; 19383.97] |
| fcfs | 22758.62 | [22605.68; 22911.56] |
| epa | 22759.30 | [22606.34; 22912.27] |
| round_robin | 28582.14 | [28403.03; 28761.26] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 18.03%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 3495.65 | [3483.95; 3507.35] |
| epa | 3495.65 | [3483.95; 3507.35] |
| prioridade | 3495.66 | [3483.96; 3507.36] |
| round_robin | 10486.67 | [10446.81; 10526.53] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa / fcfs**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou resultado igual. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 526.07 | [522.40; 529.73] |
| fcfs | 526.11 | [522.45; 529.78] |
| round_robin | 600.93 | [599.14; 602.73] |
| prioridade | 719.28 | [708.42; 730.15] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 0.01%. Os IC95% do EPA e desse clássico se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 80.55 | [80.05; 81.04] |
| epa | 42.85 | [41.69; 44.02] |
| fcfs | 42.85 | [41.68; 44.02] |
| prioridade | 13.32 | [12.78; 13.87] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 46.79%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Síntese do EPA

| Cenário | Métrica | Melhor geral | Melhor clássico | Vantagem do EPA | Sobreposição IC95% |
|---|---|---|---|---:|---|
| equilibrado | Turnaround médio | prioridade | prioridade | -22.21% | não |
| equilibrado | Trocas de contexto | epa / fcfs / prioridade | fcfs / prioridade | 0.00% | sim |
| equilibrado | Slowdown médio | epa | fcfs | 0.01% | sim |
| equilibrado | Jain do slowdown | round_robin | round_robin | -48.51% | não |
| io_bound | Turnaround médio | prioridade | prioridade | -45.40% | não |
| io_bound | Trocas de contexto | epa / fcfs / prioridade / round_robin | fcfs / prioridade / round_robin | 0.00% | sim |
| io_bound | Slowdown médio | prioridade | prioridade | -37.88% | não |
| io_bound | Jain do slowdown | fcfs / round_robin | fcfs / round_robin | -0.02% | sim |
| cpu_bound | Turnaround médio | prioridade | prioridade | -11.53% | não |
| cpu_bound | Trocas de contexto | epa / fcfs / prioridade | fcfs / prioridade | 0.00% | sim |
| cpu_bound | Slowdown médio | epa | fcfs | 0.00% | sim |
| cpu_bound | Jain do slowdown | round_robin | round_robin | -17.55% | não |
| prioridades_desbalanceadas | Turnaround médio | prioridade | prioridade | -18.03% | não |
| prioridades_desbalanceadas | Trocas de contexto | epa / fcfs | fcfs | 0.00% | sim |
| prioridades_desbalanceadas | Slowdown médio | epa | fcfs | 0.01% | sim |
| prioridades_desbalanceadas | Jain do slowdown | round_robin | round_robin | -46.79% | não |
