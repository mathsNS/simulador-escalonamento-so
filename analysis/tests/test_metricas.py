import importlib.util
import math
import unittest
from pathlib import Path

SPEC = importlib.util.spec_from_file_location(
    "analisar", Path(__file__).parents[1] / "analisar.py"
)
analisar = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(analisar)


class MetricTests(unittest.TestCase):
    def test_jain_identical_values_is_100(self):
        self.assertAlmostEqual(analisar.jain([2.0, 2.0, 2.0]), 100.0)

    def test_process_and_run_metrics(self):
        processes = [
            {"seed": "1", "cenario": "x", "algoritmo": "a", "pid": "1", "chegada": "0", "termino": "10", "cpu_total": "5", "io_total": "0"},
            {"seed": "1", "cenario": "x", "algoritmo": "a", "pid": "2", "chegada": "2", "termino": "14", "cpu_total": "4", "io_total": "2"},
        ]
        runs = [{
            "seed": "1", "cenario": "x", "algoritmo": "a",
            "num_processos": "2", "total_seeds": "1", "quantum": "4",
            "custo_troca": "1", "trocas_contexto": "7",
        }]
        detailed, per_run = analisar.calculate(processes, runs)
        self.assertEqual([row["slowdown"] for row in detailed], [2.0, 2.0])
        self.assertEqual(per_run[0]["turnaround_medio"], 11.0)
        self.assertEqual(per_run[0]["trocas_contexto"], 7)
        self.assertEqual(per_run[0]["jain_slowdown_pct"], 100.0)

    def test_ic95_uses_seeds_as_samples(self):
        # Cada linha representa uma seed independente. O teste confirma que o
        # denominador do erro-padrão é a raiz do número de seeds.
        rows = [
            {"seed": str(seed), "cenario": "x", "algoritmo": "a", "turnaround_medio": value,
             "num_processos": 2, "total_seeds": 2, "quantum": 4, "custo_troca": 1,
             "trocas_contexto": value, "slowdown_medio": value, "jain_slowdown_pct": value}
            for seed, value in ((1, 10.0), (2, 14.0))
        ]
        result = analisar.summarize(rows)[0]
        self.assertEqual(result["n_seeds"], 2)
        self.assertAlmostEqual(result["media"], 12.0)
        expected = 1.96 * math.sqrt(8) / math.sqrt(2)
        self.assertAlmostEqual(result["margem_ic95"], expected)


if __name__ == "__main__":
    unittest.main()
