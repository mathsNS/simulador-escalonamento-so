#!/usr/bin/env python3
"""Gera tabelas comparativas a partir do resumo com IC95%."""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path


# Direção considerada favorável em cada métrica.
# Menor turnaround, trocas e slowdown são desejáveis; maior Jain é desejável.
METRICS = {
    "turnaround_medio": ("Turnaround médio", "menor", "min"),
    "trocas_contexto": ("Trocas de contexto", "menor", "min"),
    "slowdown_medio": ("Slowdown médio", "menor", "min"),
    "jain_slowdown_pct": ("Jain do slowdown", "maior", "max"),
}

SCENARIO_ORDER = (
    "equilibrado",
    "io_bound",
    "cpu_bound",
    "prioridades_desbalanceadas",
)

ALGORITHM_ORDER = ("fcfs", "round_robin", "prioridade", "epa")
CLASSICAL = {"fcfs", "round_robin", "prioridade"}
REQUIRED_COLUMNS = {
    "cenario", "algoritmo", "metrica", "n_seeds", "media",
    "ic95_inferior", "ic95_superior",
}


def read_summary(path: Path) -> list[dict]:
    """Lê e valida o resumo produzido por analisar.py."""
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        missing = REQUIRED_COLUMNS - set(reader.fieldnames or [])
        if missing:
            raise ValueError(f"colunas ausentes: {', '.join(sorted(missing))}")
        rows = []
        seen = set()
        for raw in reader:
            key = (raw["cenario"], raw["algoritmo"], raw["metrica"])
            if key in seen:
                raise ValueError(f"resultado duplicado: {key}")
            seen.add(key)
            if raw["metrica"] not in METRICS:
                continue
            rows.append({
                "cenario": raw["cenario"],
                "algoritmo": raw["algoritmo"],
                "metrica": raw["metrica"],
                "n_seeds": int(raw["n_seeds"]),
                "media": float(raw["media"]),
                "ic95_inferior": float(raw["ic95_inferior"]),
                "ic95_superior": float(raw["ic95_superior"]),
            })
    if not rows:
        raise ValueError("arquivo sem métricas reconhecidas")
    return rows


def intervals_overlap(first: dict, second: dict) -> bool:
    """Informa se dois IC95% possuem ao menos um ponto em comum."""
    return max(first["ic95_inferior"], second["ic95_inferior"]) <= min(
        first["ic95_superior"], second["ic95_superior"]
    )


def ordered(rows: list[dict], direction: str) -> list[dict]:
    """Ordena do resultado mais favorável ao menos favorável."""
    return sorted(rows, key=lambda row: row["media"], reverse=direction == "max")


def tied(first: float, second: float) -> bool:
    """Considera empate apenas diferenças desprezíveis de arredondamento."""
    return math.isclose(first, second, rel_tol=1e-12, abs_tol=1e-12)


def best_names(rows: list[dict], direction: str) -> str:
    """Retorna todos os algoritmos empatados na melhor média."""
    ranking = ordered(rows, direction)
    best_value = ranking[0]["media"]
    return " / ".join(
        item["algoritmo"] for item in ranking if tied(item["media"], best_value)
    )


def ranking_text(rows: list[dict], direction: str) -> str:
    """Formata o ranking usando '=' entre médias empatadas."""
    ranking = ordered(rows, direction)
    groups: list[list[str]] = []
    values: list[float] = []
    for item in ranking:
        if values and tied(item["media"], values[-1]):
            groups[-1].append(item["algoritmo"])
        else:
            values.append(item["media"])
            groups.append([item["algoritmo"]])
    return " > ".join(" = ".join(group) for group in groups)


def relative_advantage(epa: float, reference: float, direction: str) -> float:
    """Calcula vantagem percentual do EPA; positivo significa EPA favorável."""
    if reference == 0:
        return 0.0
    if direction == "min":
        return 100.0 * (reference - epa) / abs(reference)
    return 100.0 * (epa - reference) / abs(reference)


def build_comparisons(rows: list[dict]) -> list[dict]:
    """Cria uma linha comparativa para cada cenário e métrica."""
    groups = defaultdict(list)
    for row in rows:
        groups[(row["cenario"], row["metrica"])].append(row)

    comparisons = []
    for (scenario, metric), items in groups.items():
        title, favorable, direction = METRICS[metric]
        by_algorithm = {item["algoritmo"]: item for item in items}
        missing = set(ALGORITHM_ORDER) - set(by_algorithm)
        if missing:
            raise ValueError(
                f"{scenario}/{metric}: algoritmos ausentes: {', '.join(sorted(missing))}"
            )
        epa = by_algorithm["epa"]
        best_classical = ordered(
            [item for item in items if item["algoritmo"] in CLASSICAL], direction
        )[0]
        advantage = relative_advantage(
            epa["media"], best_classical["media"], direction
        )
        comparisons.append({
            "cenario": scenario,
            "metrica": metric,
            "titulo": title,
            "sentido_favoravel": favorable,
            "melhor_geral": best_names(items, direction),
            "ranking": ranking_text(items, direction),
            "media_epa": epa["media"],
            "ic95_epa_inferior": epa["ic95_inferior"],
            "ic95_epa_superior": epa["ic95_superior"],
            "melhor_classico": best_names(
                [item for item in items if item["algoritmo"] in CLASSICAL], direction
            ),
            "media_melhor_classico": best_classical["media"],
            "vantagem_epa_pct": advantage,
            "ic95_sobrepoe_melhor_classico": (
                "sim" if intervals_overlap(epa, best_classical) else "não"
            ),
        })
    return sorted(
        comparisons,
        key=lambda row: (
            SCENARIO_ORDER.index(row["cenario"]),
            list(METRICS).index(row["metrica"]),
        ),
    )


def write_csv(path: Path, rows: list[dict]) -> None:
    """Grava a tabela comparativa em formato reutilizável."""
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def fmt(value: float) -> str:
    """Formata números do relatório com duas casas decimais."""
    return f"{value:.2f}"


def write_markdown(path: Path, rows: list[dict], comparisons: list[dict]) -> None:
    """Produz o relatório comparativo em Markdown."""
    lookup = {
        (row["cenario"], row["metrica"], row["algoritmo"]): row for row in rows
    }
    lines = [
        "# Análise comparativa dos algoritmos",
        "",
        "Os valores abaixo são médias entre seeds. Os intervalos apresentados "
        "são IC95% calculados com as seeds como repetições independentes.",
        "",
        "> A sobreposição de IC95% é apenas um auxílio visual e não constitui, "
        "isoladamente, um teste formal de significância estatística.",
        "",
    ]

    for scenario in SCENARIO_ORDER:
        lines.extend([f"## Cenário: {scenario}", ""])
        for metric, (title, favorable, direction) in METRICS.items():
            comp = next(
                item for item in comparisons
                if item["cenario"] == scenario and item["metrica"] == metric
            )
            lines.extend([
                f"### {title}",
                "",
                "| Algoritmo | Média | IC95% |",
                "|---|---:|---:|",
            ])
            metric_rows = [
                lookup[(scenario, metric, algorithm)] for algorithm in ALGORITHM_ORDER
            ]
            for item in ordered(metric_rows, direction):
                lines.append(
                    f"| {item['algoritmo']} | {fmt(item['media'])} | "
                    f"[{fmt(item['ic95_inferior'])}; {fmt(item['ic95_superior'])}] |"
                )
            advantage = comp["vantagem_epa_pct"]
            if advantage > 0:
                result = f"vantagem de {fmt(advantage)}%"
            elif advantage < 0:
                result = f"desvantagem de {fmt(abs(advantage))}%"
            else:
                result = "resultado igual"
            lines.extend([
                "",
                f"O resultado favorável é o {favorable}. O melhor resultado geral "
                f"foi de **{comp['melhor_geral']}**. Em relação ao melhor clássico "
                f"(**{comp['melhor_classico']}**), o EPA apresentou {result}. Os "
                f"IC95% do EPA e desse clássico "
                f"{'se sobrepõem' if comp['ic95_sobrepoe_melhor_classico'] == 'sim' else 'não se sobrepõem'}.",
                "",
            ])

    lines.extend([
        "## Síntese do EPA",
        "",
        "| Cenário | Métrica | Melhor geral | Melhor clássico | Vantagem do EPA | Sobreposição IC95% |",
        "|---|---|---|---|---:|---|",
    ])
    for item in comparisons:
        lines.append(
            f"| {item['cenario']} | {item['titulo']} | {item['melhor_geral']} | "
            f"{item['melhor_classico']} | {fmt(item['vantagem_epa_pct'])}% | "
            f"{item['ic95_sobrepoe_melhor_classico']} |"
        )

    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--resumo", type=Path, default=Path("resultados/resumo_ic95.csv")
    )
    parser.add_argument(
        "--saida", type=Path, default=Path("resultados/analise_comparativa.md")
    )
    parser.add_argument(
        "--tabela", type=Path, default=Path("resultados/comparacoes.csv")
    )
    args = parser.parse_args()

    rows = read_summary(args.resumo)
    comparisons = build_comparisons(rows)
    write_csv(args.tabela, comparisons)
    write_markdown(args.saida, rows, comparisons)
    print(f"Tabela comparativa: {args.tabela.resolve()}")
    print(f"Relatorio comparativo: {args.saida.resolve()}")


if __name__ == "__main__":
    main()

