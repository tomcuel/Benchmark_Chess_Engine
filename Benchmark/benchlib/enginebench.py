"""Mesure des performances internes du moteur avec sa commande « bench » (positions fixes, profondeur fixe)."""

import re
import subprocess

BENCH_LINE = re.compile(
    r"^bench depth (\d+) positions (\d+) nodes (\d+) qnodes (\d+) tthits (\d+) cutoffs (\d+) "
    r"firstmovecutoffs (\d+)% time (\d+) nps (\d+)", re.MULTILINE)


def run_bench(engine_cmd, depth, timeout=600):
    """Renvoie les compteurs de la recherche, ou None si le binaire ne connaît pas la commande."""
    try:
        completed = subprocess.run([engine_cmd, "bench", str(depth)], capture_output=True, text=True,
                                   timeout=timeout, stdin=subprocess.DEVNULL)
    except (OSError, subprocess.TimeoutExpired):
        return None
    match = BENCH_LINE.search(completed.stdout)
    if not match:
        return None
    depth, positions, nodes, qnodes, tthits, cutoffs, first, time_ms, nps = (int(value) for value in match.groups())
    return {
        "depth": depth, "positions": positions, "nodes": nodes, "quiescence_nodes": qnodes,
        "transposition_hits": tthits, "beta_cutoffs": cutoffs, "first_move_cutoff_percent": first,
        "time_ms": time_ms, "nps": nps,
    }
