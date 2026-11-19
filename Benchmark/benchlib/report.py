"""Génération du rapport Markdown d'une campagne et suivi de l'historique."""

import json
import math
from pathlib import Path

HISTORY_FILE = "history.jsonl"


def _number(value, digits=0):
    """Nombre avec espace insécable fine comme séparateur de milliers."""
    if value is None:
        return "–"
    text = f"{value:,.{digits}f}".replace(",", " ")
    return text.replace(".", ",")


def _elo(value):
    if value is None:
        return "–"
    if math.isinf(value):
        return "+∞" if value > 0 else "−∞"
    return f"{value:+.0f}".replace("-", "−")


def _percent(value):
    return f"{value * 100:.1f} %".replace(".", ",")


def _rating_text(estimate):
    if not estimate:
        return "–"
    if estimate.get("elo") is None:
        sign = "inférieur à" if estimate["bound"] == "max" else "supérieur à"
        return f"{sign} {estimate['bound_value']}"
    return f"{estimate['elo']:.0f} ± {estimate['margin']:.0f}"


def _options_text(options):
    return ", ".join(f"{key} = {str(value).lower() if isinstance(value, bool) else value}" for key, value in options.items())


def load_history(results_dir):
    path = Path(results_dir) / HISTORY_FILE
    if not path.is_file():
        return []
    entries = []
    for line in path.read_text(encoding="utf-8").splitlines():
        try:
            entries.append(json.loads(line))
        except json.JSONDecodeError:
            continue
    return entries


def save_history(results_dir, entry):
    """Ajoute (ou remplace) la ligne de la campagne dans l'historique."""
    entries = [item for item in load_history(results_dir) if item.get("directory") != entry["directory"]]
    entries.append(entry)
    entries.sort(key=lambda item: item.get("date", ""))
    path = Path(results_dir) / HISTORY_FILE
    path.write_text("".join(json.dumps(item, ensure_ascii=False) + "\n" for item in entries), encoding="utf-8")


def history_entry(campaign):
    summary, config = campaign["summary"], campaign["config"]
    entry = {
        "directory": campaign["directory"], "date": campaign["date"], "name": config["name"], "mode": config["mode"],
        "engine_version": config["engine"]["version"], "engine_sha256": config["engine"]["sha256"][:12],
        "time_control": config["time_control"], "games": summary["counted_games"], "complete": summary["complete"],
        "score": summary["score"],
    }
    if config["mode"] == "gauntlet":
        estimate = summary.get("rating") or {}
        entry.update({"elo": estimate.get("elo"), "margin": estimate.get("margin")})
    else:
        entry.update({"elo_difference": summary["elo_difference"].get("elo"), "verdict": summary["verdict"]["label"],
                      "baseline_version": config["baseline"]["version"]})
    search, bench = summary.get("search"), campaign.get("bench")
    entry["average_nps"] = search["average_nps"] if search else None
    entry["average_depth"] = search["average_depth"] if search else None
    entry["bench_nodes"] = bench["nodes"] if bench else None
    return entry


def write_report(campaign, history, path):
    Path(path).write_text(render(campaign, history), encoding="utf-8")


def render(campaign, history):
    config, summary = campaign["config"], campaign["summary"]
    engine = config["engine"]
    lines = [f"# Rapport de benchmark : {config['name']}", ""]

    if not summary["complete"]:
        lines += [f"> **Campagne incomplète** : {summary['played_games']} parties jouées sur {summary['planned_games']} prévues. "
                  f"Reprise possible avec `python benchmark.py --resume {campaign['directory']}`.", ""]

    # ----- résultat principal
    lines += ["## Résultat", "", "| Métrique | Résultat |", "|---|---:|"]
    if config["mode"] == "gauntlet":
        lines.append(f"| Elo estimé (intervalle de confiance à 95 %) | {_rating_text(summary['rating'])} |")
    else:
        difference = summary["elo_difference"]
        lines.append(f"| Verdict | **{summary['verdict']['label']}** |")
        lines.append(f"| Écart Elo (nouvelle version − référence) | {_elo(difference['elo'])} "
                     f"[{_elo(difference['elo_low'])} ; {_elo(difference['elo_high'])}] |")
        if summary.get("sprt"):
            sprt = summary["sprt"]
            lines.append(f"| Rapport de vraisemblance (LLR) | {sprt['llr']:.2f} (bornes {sprt['lower']:.2f} et {sprt['upper']:.2f}) |".replace(".", ","))
    counted = summary["counted_games"]
    lines += [
        f"| Parties comptées | {counted} |",
        f"| Victoires | {summary['wins']} ({_percent(summary['wins'] / counted) if counted else '–'}) |",
        f"| Nulles | {summary['draws']} ({_percent(summary['draws'] / counted) if counted else '–'}) |",
        f"| Défaites | {summary['losses']} ({_percent(summary['losses'] / counted) if counted else '–'}) |",
        f"| Score | {_percent(summary['score']) if counted else '–'} |",
    ]
    search = summary.get("search")
    if search:
        lines += [f"| NPS moyen | {_number(search['average_nps'])} |",
                  f"| Temps moyen de réflexion | {_number(search['average_time_ms'])} ms |",
                  f"| Profondeur moyenne | {_number(search['average_depth'], 1)} |"]
    lines.append("")
    if config["mode"] == "sprt":
        lines += [summary["verdict"]["detail"], ""]
    for warning in summary.get("warnings", []):
        lines.append(f"- ⚠️ {warning}")
    if summary.get("warnings"):
        lines.append("")

    # ----- contexte
    lines += ["## Campagne", "", "| Paramètre | Valeur |", "|---|---|",
              f"| Date | {campaign['date']} |",
              f"| Mode | {'moteur contre Stockfish bridé' if config['mode'] == 'gauntlet' else 'nouvelle version contre version de référence (SPRT)'} |",
              f"| Moteur | {engine['name']} |",
              f"| Version du moteur | {engine['version']} (empreinte `{engine['sha256'][:12]}`) |",
              f"| Options du moteur | {_options_text(engine['options'])} |"]
    if config["mode"] == "sprt":
        baseline = config["baseline"]
        sprt = config["sprt"]
        lines += [f"| Version de référence | {baseline['version']} (empreinte `{baseline['sha256'][:12]}`) |",
                  f"| Options de la référence | {_options_text(baseline['options'])} |",
                  f"| Hypothèses du SPRT | gain nul ou ≤ {sprt['elo0']} Elo contre gain ≥ {sprt['elo1']} Elo, risques {sprt['alpha']} et {sprt['beta']} |",
                  f"| Ouvertures | imposées aux deux versions ({config['openings']['plies']} demi-coups tirés du livre), livre des moteurs coupé |"]
    else:
        stockfish = config["stockfish"]
        lines += [f"| Adversaire | Stockfish avec `UCI_LimitStrength` ({_options_text(stockfish['options'])}) |",
                  f"| Paliers | {', '.join(str(level) for level in stockfish['elo_levels'])} |"]
    planned = f"{summary['planned_games']}"
    if summary.get("stopped_by_sprt") and summary["played_games"] < summary["planned_games"]:
        planned += f" au maximum (arrêt du test séquentiel après {summary['played_games']})"
    lines += [f"| Cadence | {config['time_control']} (secondes + incrément par coup) |",
              f"| Parties prévues | {planned} |",
              f"| Parties en parallèle | {config['concurrency']} |",
              f"| Fichier de configuration | `{config['config_file']}` |", ""]

    # ----- détail par adversaire
    lines += ["## Résultats par adversaire", "",
              "| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |",
              "|---|---:|---:|---:|---:|---:|---:|---:|---:|"]
    for name, stats in summary["opponents"].items():
        if not stats["games"]:
            lines.append(f"| {name} | 0 | – | – | – | – | – | – | – |")
            continue
        white = f"{_number(stats['white_points'], 1)} / {stats['white_games']}"
        black = f"{_number(stats['black_points'], 1)} / {stats['black_games']}"
        elo = f"{_elo(stats['elo'])} [{_elo(stats['elo_low'])} ; {_elo(stats['elo_high'])}]"
        lines.append(f"| {name} | {stats['games']} | {stats['wins']} | {stats['draws']} | {stats['losses']} | "
                     f"{_percent(stats['score'])} | {white} | {black} | {elo} |")
    lines.append("")
    if config["mode"] == "gauntlet":
        lines += [f"Elo estimé avec les blancs : {_rating_text(summary['rating_white'])}. "
                  f"Avec les noirs : {_rating_text(summary['rating_black'])}.", "",
                  "L'Elo global est le classement qui rend la somme des scores attendus égale à la somme des scores obtenus, "
                  "tous paliers confondus. Il dépend de la cadence, de la machine et de l'étalonnage de Stockfish bridé : "
                  "ce n'est pas un classement officiel.", ""]

    # ----- fins de partie
    lines += ["## Fins de partie", "", "| Type | Parties |", "|---|---:|"]
    lines += [f"| {label} | {count} |" for label, count in summary["terminations"].items()]
    lines.append("")
    engine_losses, opponent_losses = summary["engine_technical_losses"], summary["opponent_technical_losses"]
    if engine_losses:
        lines.append(f"**{len(engine_losses)} défaite(s) technique(s) du moteur testé** (comptées comme des défaites) :")
        lines += [f"- contre {item['opponent']} : {item['reason']}" for item in engine_losses]
    else:
        lines.append("Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).")
    if opponent_losses:
        lines.append("")
        lines.append(f"{len(opponent_losses)} partie(s) écartée(s) des résultats, perdue(s) par l'adversaire pour une raison technique :")
        lines += [f"- {item['opponent']} : {item['reason']}" for item in opponent_losses]
    if summary["unfinished_games"]:
        lines += ["", f"{summary['unfinished_games']} partie(s) sans résultat, ignorée(s)."]
    if summary.get("replayed_games"):
        lines += ["", f"{summary['replayed_games']} partie(s) rejouée(s) après une reprise : seule la dernière version de chacune est comptée."]
    lines.append("")

    # ----- performances
    lines += ["## Performances de la recherche", ""]
    if search:
        lines += ["Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :", "",
                  "| Indicateur | Valeur |", "|---|---:|",
                  f"| Coups réfléchis | {_number(search['searched_moves'])} |",
                  f"| Coups tirés du livre | {_number(search['book_moves'])} |",
                  f"| NPS moyen | {_number(search['average_nps'])} |",
                  f"| Nœuds par coup | {_number(search['average_nodes'])} |",
                  f"| Profondeur moyenne | {_number(search['average_depth'], 1)} |",
                  f"| Temps moyen par coup | {_number(search['average_time_ms'])} ms |",
                  f"| Occupation de la table de hachage | {_number(search['average_hashfull'] / 10, 1)} % |", ""]
    bench = campaign.get("bench")
    if bench:
        lines += [f"Mesures sur {bench['positions']} positions fixes à profondeur {bench['depth']} (commande `bench`). "
                  "Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :", "",
                  "| Indicateur | Valeur |", "|---|---:|",
                  f"| Nœuds explorés | {_number(bench['nodes'])} |",
                  f"| dont recherche de quiescence | {_number(bench['quiescence_nodes'])} ({_percent(bench['quiescence_nodes'] / bench['nodes'])}) |",
                  f"| Coupures alpha-bêta | {_number(bench['beta_cutoffs'])} |",
                  f"| Coupures dès le premier coup essayé | {bench['first_move_cutoff_percent']} % |",
                  f"| Positions retrouvées dans la table de transposition | {_number(bench['transposition_hits'])} |",
                  f"| Temps | {_number(bench['time_ms'])} ms |",
                  f"| NPS | {_number(bench['nps'])} |", ""]
    baseline_bench = campaign.get("baseline_bench")
    if bench and baseline_bench:
        change = (bench["nodes"] - baseline_bench["nodes"]) / baseline_bench["nodes"] * 100
        lines += [f"Version de référence sur les mêmes positions : {_number(baseline_bench['nodes'])} nœuds "
                  f"({change:+.1f} % pour la nouvelle version), {_number(baseline_bench['nps'])} NPS.".replace(".", ",", 1), ""]
    if not search and not bench:
        lines += ["Aucune mesure disponible.", ""]

    # ----- comparaison
    lines += ["## Comparaison avec les campagnes précédentes", ""]
    previous = [item for item in history if item.get("directory") != campaign["directory"] and item.get("mode") == config["mode"]]
    if not previous:
        lines += ["Aucune campagne précédente du même mode.", ""]
    else:
        current = history_entry(campaign)
        rows = previous[-6:] + [current]
        if config["mode"] == "gauntlet":
            lines += ["| Date | Campagne | Version | Cadence | Parties | Elo estimé | NPS moyen | Nœuds du bench |",
                      "|---|---|---|---|---:|---:|---:|---:|"]
            for item in rows:
                elo = f"{item['elo']:.0f} ± {item['margin']:.0f}" if item.get("elo") is not None else "–"
                marker = " (celle-ci)" if item is current else ""
                lines.append(f"| {item['date']} | {item['name']}{marker} | {item['engine_version']} | {item['time_control']} | "
                             f"{item['games']} | {elo} | {_number(item.get('average_nps'))} | {_number(item.get('bench_nodes'))} |")
            same = [item for item in previous if item["time_control"] == config["time_control"] and item.get("elo") is not None]
            if same and current.get("elo") is not None:
                last = same[-1]
                delta = current["elo"] - last["elo"]
                combined = math.hypot(current["margin"], last["margin"])
                judgement = "écart significatif" if abs(delta) > combined else "écart dans la marge d'erreur"
                lines += ["", f"Par rapport à « {last['name']} » ({last['date']}, même cadence) : {_elo(delta)} Elo, "
                              f"marge combinée ± {combined:.0f} ({judgement}). Pour comparer deux versions, le mode SPRT est plus précis."]
        else:
            lines += ["| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |", "|---|---|---|---|---:|---:|---|"]
            for item in rows:
                marker = " (celle-ci)" if item is current else ""
                lines.append(f"| {item['date']} | {item['name']}{marker} | {item['engine_version']} | {item.get('baseline_version', '–')} | "
                             f"{item['games']} | {_elo(item.get('elo_difference'))} | {item.get('verdict', '–')} |")
        lines.append("")

    lines += ["## Fichiers", "",
              "- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup",
              "- `summary.json` : les chiffres de ce rapport",
              "- `campaign.json` : la configuration exacte de la campagne",
              "- `fastchess_output.txt` : la sortie de Fastchess", ""]
    return "\n".join(lines)
