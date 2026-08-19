"""Calcula métricas, IC95% e gráficos do simulador de escalonamento.

Este programa recebe dois CSVs produzidos pelo simulador:

1. um arquivo com os resultados individuais dos processos;
2. um arquivo com dados globais de cada execução, como trocas de contexto.

As métricas são calculadas primeiro dentro de cada execução (seed). Depois, os
resultados das seeds são agregados para obter a média e o IC95%.
"""

from __future__ import annotations

import argparse
import csv
import math
import statistics
from collections import defaultdict
from pathlib import Path

# Colunas obrigatórias do arquivo que possui uma linha por processo.
PROCESS_COLUMNS = {
    "seed", "cenario", "algoritmo", "pid", "chegada", "termino", "cpu_total", "io_total"
}
# Colunas obrigatórias do arquivo que possui uma linha por execução.
RUN_COLUMNS = {"seed", "cenario", "algoritmo", "trocas_contexto"}

# Campos que identificam unicamente uma execução experimental.
KEY = ("seed", "cenario", "algoritmo")


def read_csv(path: Path, required: set[str]) -> list[dict[str, str]]:
    """Lê um CSV e verifica se ele contém as colunas e os dados esperados."""
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        missing = required - set(reader.fieldnames or [])
        if missing:
            raise ValueError(f"{path}: colunas ausentes: {', '.join(sorted(missing))}")
        rows = list(reader)
    if not rows:
        raise ValueError(f"{path}: arquivo sem dados")
    return rows


def write_csv(path: Path, fieldnames: list[str], rows: list[dict]) -> None:
    """Grava uma lista de dicionários em CSV, criando a pasta de destino."""
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def jain(values: list[float]) -> float:
    """Calcula o índice de Jain e devolve o resultado como porcentagem.

    Um valor próximo de 100 indica que os processos tiveram slowdowns
    semelhantes. Valores menores indicam maior desigualdade.
    """
    if not values or any(value <= 0 for value in values):
        raise ValueError("slowdowns usados no índice de Jain devem ser positivos")
    return 100.0 * sum(values) ** 2 / (len(values) * sum(value * value for value in values))


def calculate(process_rows: list[dict[str, str]], run_rows: list[dict[str, str]]):
    """Calcula métricas por processo e por execução.

    Retorna duas tabelas:
    - detalhes de cada processo, incluindo turnaround e slowdown;
    - resumo de cada execução/seed, incluindo o índice de Jain.
    """
    detailed = []
    # Agrupa os processos que pertencem à mesma execução experimental.
    grouped: dict[tuple[str, str, str], list[dict]] = defaultdict(list)
    seen_processes = set()

    for raw in process_rows:
        # A combinação abaixo não pode aparecer duas vezes para o mesmo PID.
        key = tuple(raw[name].strip() for name in KEY)
        process_id = (*key, raw["pid"].strip())
        if process_id in seen_processes:
            raise ValueError(f"processo duplicado: {process_id}")
        seen_processes.add(process_id)
        chegada = float(raw["chegada"])
        termino = float(raw["termino"])
        cpu = float(raw["cpu_total"])
        io = float(raw["io_total"])
        # Turnaround: tempo total entre a chegada e o término do processo.
        turnaround = termino - chegada

        # Tempo mínimo ideal: processo sem espera na fila de prontos.
        # O custo de troca de contexto não é somado aqui.
        ideal = cpu + io
        if chegada < 0 or termino < chegada or cpu <= 0 or io < 0 or ideal <= 0:
            raise ValueError(f"tempos inválidos no processo {process_id}")
        item = {
            "seed": key[0], "cenario": key[1], "algoritmo": key[2], "pid": raw["pid"],
            "chegada": chegada, "termino": termino, "cpu_total": cpu, "io_total": io,
            # Slowdown compara o turnaround observado com o tempo mínimo ideal.
            "turnaround": turnaround, "tempo_ideal": ideal, "slowdown": turnaround / ideal,
        }
        if item["slowdown"] < 1.0 - 1e-9:
            raise ValueError(f"slowdown menor que 1 no processo {process_id}; verifique os tempos")
        detailed.append(item)
        grouped[key].append(item)

    # As trocas de contexto são uma propriedade da execução inteira, não de um
    # processo. Por isso vêm no segundo CSV e são associadas pela chave KEY.
    switches = {}
    for raw in run_rows:
        key = tuple(raw[name].strip() for name in KEY)
        if key in switches:
            raise ValueError(f"execução duplicada: {key}")
        value = int(raw["trocas_contexto"])
        if value < 0:
            raise ValueError(f"trocas de contexto negativas: {key}")
        switches[key] = value
    # Toda execução presente em um CSV precisa estar presente no outro.
    if set(grouped) != set(switches):
        missing_runs = sorted(set(grouped) - set(switches))
        missing_processes = sorted(set(switches) - set(grouped))
        raise ValueError(
            f"chaves incompatíveis; sem execução={missing_runs}; sem processos={missing_processes}"
        )

    # Consolida as informações dos processos dentro de cada execução/seed.
    per_run = []
    for key in sorted(grouped):
        items = grouped[key]
        slowdowns = [item["slowdown"] for item in items]
        per_run.append({
            "seed": key[0], "cenario": key[1], "algoritmo": key[2],
            "num_processos": len(items),
            "turnaround_medio": statistics.fmean(item["turnaround"] for item in items),
            "trocas_contexto": switches[key],
            "slowdown_medio": statistics.fmean(slowdowns),
            "jain_slowdown_pct": jain(slowdowns),
        })
    return detailed, per_run


def summarize(per_run: list[dict]) -> list[dict]:
    """Agrega os resultados entre seeds e calcula média e IC95%.

    A unidade amostral é a seed. Os processos de uma mesma execução não são
    tratados como experimentos independentes.
    """
    metrics = ("turnaround_medio", "trocas_contexto", "slowdown_medio", "jain_slowdown_pct")
    groups: dict[tuple[str, str], list[dict]] = defaultdict(list)
    for row in per_run:
        groups[(row["cenario"], row["algoritmo"])].append(row)
    summary = []
    for (scenario, algorithm), rows in sorted(groups.items()):
        seeds = [str(row["seed"]) for row in rows]
        if len(seeds) != len(set(seeds)):
            raise ValueError(f"seeds repetidas em {scenario}/{algorithm}")
        for metric in metrics:
            values = [float(row[metric]) for row in rows]
            mean = statistics.fmean(values)
            # statistics.stdev calcula o desvio-padrão amostral (denominador n-1).
            sd = statistics.stdev(values) if len(values) > 1 else 0.0

            # Fórmula pedida no enunciado: média +/- 1,96 * s / sqrt(n).
            margin = 1.96 * sd / math.sqrt(len(values))
            summary.append({
                "cenario": scenario, "algoritmo": algorithm, "metrica": metric,
                "n_seeds": len(values), "media": mean, "desvio_padrao": sd,
                "ic95_inferior": mean - margin, "ic95_superior": mean + margin,
                "margem_ic95": margin,
            })
    return summary


def svg_escape(text: str) -> str:
    """Escapa caracteres que possuem significado especial em XML/SVG."""
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def make_svg(rows: list[dict], metric: str, output: Path) -> None:
    """Cria um gráfico SVG de médias com barras de erro do IC95%.

    Cada grupo no eixo horizontal representa um cenário. As cores representam
    os algoritmos. O ponto é a média e a haste vertical é o IC95%.
    """
    selected = [row for row in rows if row["metrica"] == metric]
    scenarios = sorted({row["cenario"] for row in selected})
    algorithms = sorted({row["algoritmo"] for row in selected})
    lookup = {(row["cenario"], row["algoritmo"]): row for row in selected}
    width, height = 1100, 620
    left, right, top, bottom = 100, 30, 70, 150
    plot_w, plot_h = width - left - right, height - top - bottom
    # O maior limite superior determina a escala do eixo vertical.
    maximum = max(float(row["ic95_superior"]) for row in selected) or 1.0
    colors = ["#2563eb", "#dc2626", "#16a34a", "#9333ea", "#ea580c", "#0891b2"]
    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{width/2}" y="32" text-anchor="middle" font-family="Arial" font-size="22" font-weight="bold">{svg_escape(metric)}</text>',
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{top+plot_h}" stroke="#111"/>',
        f'<line x1="{left}" y1="{top+plot_h}" x2="{left+plot_w}" y2="{top+plot_h}" stroke="#111"/>',
    ]
    # Desenha seis marcas igualmente espaçadas no eixo vertical.
    for tick in range(6):
        value = maximum * tick / 5
        y = top + plot_h - plot_h * tick / 5
        parts.extend([
            f'<line x1="{left}" y1="{y:.1f}" x2="{left+plot_w}" y2="{y:.1f}" stroke="#e5e7eb"/>',
            f'<text x="{left-10}" y="{y+4:.1f}" text-anchor="end" font-family="Arial" font-size="12">{value:.2f}</text>',
        ])
    group_w = plot_w / max(1, len(scenarios))
    bar_w = min(34, group_w / (len(algorithms) + 1))
    # Desenha um ponto e uma barra de erro para cada algoritmo/cenário.
    for si, scenario in enumerate(scenarios):
        center = left + group_w * (si + 0.5)
        parts.append(f'<text x="{center:.1f}" y="{top+plot_h+28}" text-anchor="middle" font-family="Arial" font-size="12">{svg_escape(scenario)}</text>')
        for ai, algorithm in enumerate(algorithms):
            row = lookup.get((scenario, algorithm))
            if not row:
                continue
            mean = float(row["media"])
            low = float(row["ic95_inferior"])
            high = float(row["ic95_superior"])
            x = center + (ai - (len(algorithms)-1)/2) * bar_w
            y_mean = top + plot_h * (1 - mean / maximum)
            y_low = top + plot_h * (1 - max(0, low) / maximum)
            y_high = top + plot_h * (1 - high / maximum)
            color = colors[ai % len(colors)]
            parts.extend([
                f'<circle cx="{x:.1f}" cy="{y_mean:.1f}" r="5" fill="{color}"/>',
                f'<line x1="{x:.1f}" y1="{y_high:.1f}" x2="{x:.1f}" y2="{y_low:.1f}" stroke="{color}" stroke-width="2"/>',
                f'<line x1="{x-5:.1f}" y1="{y_high:.1f}" x2="{x+5:.1f}" y2="{y_high:.1f}" stroke="{color}"/>',
                f'<line x1="{x-5:.1f}" y1="{y_low:.1f}" x2="{x+5:.1f}" y2="{y_low:.1f}" stroke="{color}"/>',
            ])
    # Legenda das cores utilizadas para os algoritmos.
    legend_y = height - 70
    for ai, algorithm in enumerate(algorithms):
        x = left + ai * 190
        color = colors[ai % len(colors)]
        parts.extend([
            f'<circle cx="{x}" cy="{legend_y}" r="5" fill="{color}"/>',
            f'<text x="{x+12}" y="{legend_y+4}" font-family="Arial" font-size="13">{svg_escape(algorithm)}</text>',
        ])
    parts.append('</svg>')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(parts), encoding="utf-8")


def main() -> None:
    """Interpreta os argumentos, executa a análise e grava todas as saídas."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--processos", type=Path, required=True)
    parser.add_argument("--execucoes", type=Path, required=True)
    parser.add_argument("--saida", type=Path, default=Path("resultados"))
    args = parser.parse_args()
    process_rows = read_csv(args.processos, PROCESS_COLUMNS)
    run_rows = read_csv(args.execucoes, RUN_COLUMNS)
    # Etapa 1: métricas individuais e métricas de cada execução.
    detailed, per_run = calculate(process_rows, run_rows)

    # Etapa 2: agregação das execuções independentes (seeds).
    summary = summarize(per_run)

    # Etapa 3: grava tabelas que permitem reproduzir a análise.
    write_csv(args.saida / "metricas_processos.csv", list(detailed[0]), detailed)
    write_csv(args.saida / "metricas_execucoes.csv", list(per_run[0]), per_run)
    write_csv(args.saida / "resumo_ic95.csv", list(summary[0]), summary)
    # Etapa 4: gera um gráfico com IC95% para cada métrica avaliada.
    for metric in ("turnaround_medio", "trocas_contexto", "slowdown_medio", "jain_slowdown_pct"):
        make_svg(summary, metric, args.saida / "graficos" / f"{metric}.svg")
    print(f"Análise concluída: {len(per_run)} execuções, {len(detailed)} processos.")
    print(f"Resultados gravados em: {args.saida.resolve()}")


if __name__ == "__main__":
    main()