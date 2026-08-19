# Análise comparativa dos algoritmos

Os valores abaixo são médias entre seeds. Os intervalos apresentados são IC95% calculados com as seeds como repetições independentes.

> A sobreposição de IC95% é apenas um auxílio visual e não constitui, isoladamente, um teste formal de significância estatística.

## Cenário: equilibrado

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 18600.48 | [18500.85; 18700.11] |
| epa | 21886.17 | [21748.65; 22023.69] |
| fcfs | 22730.91 | [22580.95; 22880.87] |
| round_robin | 28568.69 | [28388.04; 28749.34] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 17.66%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 3495.28 | [3483.57; 3506.99] |
| fcfs | 3495.68 | [3483.98; 3507.38] |
| prioridade | 3495.68 | [3483.98; 3507.38] |
| round_robin | 10483.44 | [10442.78; 10524.10] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs / prioridade**), o EPA apresentou vantagem de 0.01%. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 527.56 | [522.74; 532.39] |
| fcfs | 531.14 | [527.36; 534.92] |
| round_robin | 602.22 | [600.47; 603.98] |
| prioridade | 760.88 | [747.73; 774.04] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 0.67%. Os IC95% do EPA e desse clássico se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 79.64 | [79.04; 80.25] |
| fcfs | 41.01 | [39.67; 42.34] |
| epa | 34.27 | [32.84; 35.71] |
| prioridade | 12.94 | [12.37; 13.52] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 56.97%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Cenário: io_bound

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 9905.50 | [9876.75; 9934.26] |
| prioridade | 11104.77 | [11071.63; 11137.91] |
| fcfs | 16148.74 | [16098.26; 16199.23] |
| round_robin | 16148.74 | [16098.26; 16199.23] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou vantagem de 10.80%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 6489.28 | [6477.50; 6501.06] |
| fcfs | 6495.16 | [6483.46; 6506.86] |
| round_robin | 6495.16 | [6483.46; 6506.86] |
| prioridade | 6495.16 | [6483.46; 6506.86] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs / prioridade / round_robin**), o EPA apresentou vantagem de 0.09%. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 204.81 | [204.15; 205.46] |
| prioridade | 225.36 | [224.69; 226.03] |
| fcfs | 310.77 | [310.13; 311.41] |
| round_robin | 310.77 | [310.13; 311.41] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou vantagem de 9.12%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| fcfs | 92.81 | [92.73; 92.89] |
| round_robin | 92.81 | [92.73; 92.89] |
| prioridade | 65.35 | [65.16; 65.53] |
| epa | 62.64 | [62.44; 62.83] |

O resultado favorável é o maior. O melhor resultado geral foi de **fcfs / round_robin**. Em relação ao melhor clássico (**fcfs / round_robin**), o EPA apresentou desvantagem de 32.51%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Cenário: cpu_bound

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 17935.29 | [17857.33; 18013.26] |
| fcfs | 20001.14 | [19901.44; 20100.84] |
| epa | 20850.85 | [20745.30; 20956.40] |
| round_robin | 34231.81 | [34095.81; 34367.82] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 16.26%. Os IC95% do EPA e desse clássico não se sobrepõem.

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
| epa | 479.95 | [478.48; 481.42] |
| fcfs | 487.72 | [486.04; 489.40] |
| prioridade | 495.74 | [493.61; 497.87] |
| round_robin | 845.84 | [844.18; 847.50] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 1.59%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 94.29 | [94.23; 94.35] |
| epa | 81.31 | [81.14; 81.49] |
| fcfs | 77.73 | [77.59; 77.87] |
| prioridade | 57.04 | [56.85; 57.24] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 13.76%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Cenário: prioridades_desbalanceadas

### Turnaround médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| prioridade | 19283.28 | [19182.59; 19383.97] |
| epa | 21946.29 | [21804.14; 22088.44] |
| fcfs | 22758.62 | [22605.68; 22911.56] |
| round_robin | 28582.14 | [28403.03; 28761.26] |

O resultado favorável é o menor. O melhor resultado geral foi de **prioridade**. Em relação ao melhor clássico (**prioridade**), o EPA apresentou desvantagem de 13.81%. Os IC95% do EPA e desse clássico não se sobrepõem.

### Trocas de contexto

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 3495.23 | [3483.52; 3506.94] |
| fcfs | 3495.65 | [3483.95; 3507.35] |
| prioridade | 3495.66 | [3483.96; 3507.36] |
| round_robin | 10486.67 | [10446.81; 10526.53] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 0.01%. Os IC95% do EPA e desse clássico se sobrepõem.

### Slowdown médio

| Algoritmo | Média | IC95% |
|---|---:|---:|
| epa | 519.45 | [515.55; 523.35] |
| fcfs | 526.11 | [522.45; 529.78] |
| round_robin | 600.93 | [599.14; 602.73] |
| prioridade | 719.28 | [708.42; 730.15] |

O resultado favorável é o menor. O melhor resultado geral foi de **epa**. Em relação ao melhor clássico (**fcfs**), o EPA apresentou vantagem de 1.27%. Os IC95% do EPA e desse clássico se sobrepõem.

### Jain do slowdown

| Algoritmo | Média | IC95% |
|---|---:|---:|
| round_robin | 80.55 | [80.05; 81.04] |
| fcfs | 42.85 | [41.68; 44.02] |
| epa | 36.11 | [34.91; 37.32] |
| prioridade | 13.32 | [12.78; 13.87] |

O resultado favorável é o maior. O melhor resultado geral foi de **round_robin**. Em relação ao melhor clássico (**round_robin**), o EPA apresentou desvantagem de 55.16%. Os IC95% do EPA e desse clássico não se sobrepõem.

## Síntese do EPA

| Cenário | Métrica | Melhor geral | Melhor clássico | Vantagem do EPA | Sobreposição IC95% |
|---|---|---|---|---:|---|
| equilibrado | Turnaround médio | prioridade | prioridade | -17.66% | não |
| equilibrado | Trocas de contexto | epa | fcfs / prioridade | 0.01% | sim |
| equilibrado | Slowdown médio | epa | fcfs | 0.67% | sim |
| equilibrado | Jain do slowdown | round_robin | round_robin | -56.97% | não |
| io_bound | Turnaround médio | epa | prioridade | 10.80% | não |
| io_bound | Trocas de contexto | epa | fcfs / prioridade / round_robin | 0.09% | sim |
| io_bound | Slowdown médio | epa | prioridade | 9.12% | não |
| io_bound | Jain do slowdown | fcfs / round_robin | fcfs / round_robin | -32.51% | não |
| cpu_bound | Turnaround médio | prioridade | prioridade | -16.26% | não |
| cpu_bound | Trocas de contexto | epa / fcfs / prioridade | fcfs / prioridade | 0.00% | sim |
| cpu_bound | Slowdown médio | epa | fcfs | 1.59% | não |
| cpu_bound | Jain do slowdown | round_robin | round_robin | -13.76% | não |
| prioridades_desbalanceadas | Turnaround médio | prioridade | prioridade | -13.81% | não |
| prioridades_desbalanceadas | Trocas de contexto | epa | fcfs | 0.01% | sim |
| prioridades_desbalanceadas | Slowdown médio | epa | fcfs | 1.27% | sim |
| prioridades_desbalanceadas | Jain do slowdown | round_robin | round_robin | -55.16% | não |
