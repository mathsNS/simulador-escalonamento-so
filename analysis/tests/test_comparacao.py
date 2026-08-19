import importlib.util
import unittest
from pathlib import Path


SPEC = importlib.util.spec_from_file_location(
    "comparar_resultados", Path(__file__).parents[1] / "comparar_resultados.py"
)
comparar = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(comparar)


class ComparisonTests(unittest.TestCase):
    def test_interval_overlap(self):
        first = {"ic95_inferior": 10.0, "ic95_superior": 12.0}
        second = {"ic95_inferior": 11.0, "ic95_superior": 13.0}
        third = {"ic95_inferior": 13.1, "ic95_superior": 14.0}
        self.assertTrue(comparar.intervals_overlap(first, second))
        self.assertFalse(comparar.intervals_overlap(first, third))

    def test_advantage_for_lower_is_better(self):
        self.assertAlmostEqual(comparar.relative_advantage(90, 100, "min"), 10)

    def test_advantage_for_higher_is_better(self):
        self.assertAlmostEqual(comparar.relative_advantage(90, 80, "max"), 12.5)

    def test_tied_algorithms_are_reported_together(self):
        rows = [
            {"algoritmo": "fcfs", "media": 10.0},
            {"algoritmo": "epa", "media": 10.0},
            {"algoritmo": "rr", "media": 12.0},
        ]
        self.assertEqual(comparar.best_names(rows, "min"), "fcfs / epa")
        self.assertEqual(comparar.ranking_text(rows, "min"), "fcfs = epa > rr")


if __name__ == "__main__":
    unittest.main()
