#!/usr/bin/env python3
"""Benchmark Elo du moteur d'échecs.

Exemples :
    python benchmark.py --config configs/baseline.json --games 200
    python benchmark.py --resume results/baseline_2026-11-16_210000
    python benchmark.py --snapshot v1
    python benchmark.py --config configs/sprt.json --games 400

Une campagne charge la configuration, lance le tournoi avec Fastchess, estime l'Elo avec son
intervalle de confiance, mesure les performances de la recherche, écrit un rapport Markdown et
garde les données pour les comparaisons suivantes.
"""

import argparse
import datetime
import json
import shutil
import sys
from pathlib import Path

from benchlib import analysis, config as configuration, enginebench, report, tournament

BENCHMARK_DIR = Path(__file__).resolve().parent
RESULTS_DIR = BENCHMARK_DIR / "results"
ENGINES_DIR = BENCHMARK_DIR / "engines"
CAMPAIGN_FILE = "campaign.json"
SUMMARY_FILE = "summary.json"
REPORT_FILE = "report.md"


def snapshot_engine(label, engine_cmd):
    """Garde une copie du binaire du moteur, pour s'en servir plus tard comme version de référence."""
    source = configuration.resolve_path(engine_cmd)
    if not source.is_file():
        raise configuration.ConfigError(f"Binaire introuvable : {source}\nCompile-le avec : make -C Chess_Engine/UCI")
    destination = ENGINES_DIR / label
    if destination.exists():
        raise configuration.ConfigError(f"La version « {label} » existe déjà : {destination}")
    destination.mkdir(parents=True)
    shutil.copy2(source, destination / source.name)
    print(f"Version « {label} » enregistrée : {destination / source.name}")
    print(f"Empreinte : {configuration.file_sha256(destination / source.name)[:12]}")
    print(f'À utiliser dans une configuration : "cmd": "engines/{label}/{source.name}"')


def start_campaign(arguments):
    config = configuration.load_config(arguments.config, arguments.games)
    if arguments.name:
        config["name"] = arguments.name
    if arguments.concurrency:
        config["concurrency"] = arguments.concurrency

    now = datetime.datetime.now()
    campaign_dir = RESULTS_DIR / f"{config['name']}_{now:%Y-%m-%d_%H%M%S}"
    campaign_dir.mkdir(parents=True)
    campaign = {"directory": str(campaign_dir.relative_to(BENCHMARK_DIR)), "date": f"{now:%Y-%m-%d %H:%M}", "config": config}

    plan = tournament.plan_games(config)
    print(f"Campagne « {config['name']} » : {plan['total']} parties, cadence {config['time_control']}, "
          f"{config['concurrency']} en parallèle")
    print(f"Moteur : {config['engine']['name']} version {config['engine']['version']} ({config['engine']['sha256'][:12]})")
    if plan["total"] != config["games"]:
        print(f"({config['games']} parties demandées, arrondies à {plan['total']} pour un nombre pair par adversaire)")

    print(f"Mesure des performances de la recherche (bench à profondeur {config['bench_depth']})...")
    campaign["bench"] = enginebench.run_bench(config["engine"]["cmd"], config["bench_depth"])
    if config["mode"] == "sprt":
        campaign["baseline_bench"] = enginebench.run_bench(config["baseline"]["cmd"], config["bench_depth"])

    command = tournament.build_command(config, campaign_dir)
    campaign["fastchess_command"] = command
    _write_json(campaign_dir / CAMPAIGN_FILE, campaign)
    print(f"Dossier : {campaign_dir}")
    print("Tournoi en cours (Ctrl+C pour interrompre, la campagne pourra être reprise)...")
    return_code, interrupted = tournament.run(command, campaign_dir, plan["total"])
    return finish_campaign(campaign, campaign_dir, return_code, interrupted)


def resume_campaign(directory):
    campaign_dir = Path(directory)
    if not campaign_dir.is_absolute() and not campaign_dir.is_dir():
        campaign_dir = BENCHMARK_DIR / directory
    campaign_file = campaign_dir / CAMPAIGN_FILE
    if not campaign_file.is_file():
        raise configuration.ConfigError(f"Ce dossier ne contient pas de campagne ({CAMPAIGN_FILE} introuvable) : {campaign_dir}")
    campaign = json.loads(campaign_file.read_text(encoding="utf-8"))
    config = campaign["config"]

    summary = analysis.analyse(config, campaign_dir)
    return_code, interrupted = 0, False
    if summary["complete"]:
        print("La campagne est déjà terminée : le rapport est simplement régénéré.")
    elif not (campaign_dir / tournament.STATE_FILE).is_file():
        print("Aucun état Fastchess sauvegardé : impossible de reprendre le tournoi, le rapport porte sur les parties jouées.")
    else:
        print(f"Reprise de « {config['name']} » : {summary['played_games']} parties déjà jouées sur {summary['planned_games']}")
        return_code, interrupted = tournament.run(tournament.resume_command(config), campaign_dir,
                                                  summary["planned_games"], already_played=summary["played_games"])
    return finish_campaign(campaign, campaign_dir, return_code, interrupted)


def finish_campaign(campaign, campaign_dir, return_code, interrupted):
    config = campaign["config"]
    campaign["summary"] = summary = analysis.analyse(config, campaign_dir)
    _write_json(campaign_dir / SUMMARY_FILE, summary)

    history = report.load_history(RESULTS_DIR)
    report.write_report(campaign, history, campaign_dir / REPORT_FILE)
    if summary["counted_games"]:
        report.save_history(RESULTS_DIR, report.history_entry(campaign))

    print()
    if return_code != 0 and not interrupted:
        print(f"Fastchess s'est terminé avec le code {return_code} : voir {campaign_dir / tournament.OUTPUT_FILE}")
    if not summary["complete"]:
        print(f"Campagne incomplète : {summary['played_games']}/{summary['planned_games']} parties.")
        print(f"Pour la reprendre : python benchmark.py --resume {campaign['directory']}")
    print(f"Résultat : {summary['wins']} victoires, {summary['draws']} nulles, {summary['losses']} défaites "
          f"sur {summary['counted_games']} parties comptées")
    if config["mode"] == "gauntlet":
        estimate = summary.get("rating")
        if estimate and estimate.get("elo") is not None:
            print(f"Elo estimé : {estimate['elo']:.0f} ± {estimate['margin']:.0f} (intervalle de confiance à 95 %)")
        elif estimate:
            print("Elo estimé : hors de la plage des adversaires (tout gagné ou tout perdu)")
    else:
        difference = summary["elo_difference"]
        if difference.get("elo") is not None:
            print(f"Écart Elo : {difference['elo']:+.0f} [{difference['elo_low']:+.0f} ; {difference['elo_high']:+.0f}]")
        print(f"Verdict : {summary['verdict']['label']}")
    for warning in summary.get("warnings", []):
        print(f"Attention : {warning}")
    if summary["engine_technical_losses"]:
        print(f"Attention : {len(summary['engine_technical_losses'])} défaite(s) technique(s) du moteur (temps, coup illégal ou plantage)")
    print(f"Rapport : {campaign_dir / REPORT_FILE}")
    return 0 if summary["complete"] else 1


def _write_json(path, data):
    Path(path).write_text(json.dumps(data, indent=2, ensure_ascii=False, default=str) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description="Benchmark Elo du moteur d'échecs (tournois Fastchess, estimation Elo, rapport Markdown).")
    parser.add_argument("--config", help="fichier JSON de la campagne (ex : configs/baseline.json)")
    parser.add_argument("--games", type=int, help="nombre total de parties (remplace celui de la configuration)")
    parser.add_argument("--name", help="nom de la campagne (remplace celui de la configuration)")
    parser.add_argument("--concurrency", type=int, help="nombre de parties jouées en parallèle")
    parser.add_argument("--resume", metavar="DOSSIER", help="reprend une campagne interrompue, ou régénère son rapport")
    parser.add_argument("--snapshot", metavar="NOM", help="garde une copie du binaire actuel du moteur dans engines/NOM/")
    parser.add_argument("--engine", default=configuration.DEFAULTS["engine"]["cmd"], help="binaire à copier avec --snapshot")
    arguments = parser.parse_args()

    try:
        if arguments.snapshot:
            snapshot_engine(arguments.snapshot, arguments.engine)
            return 0
        if arguments.resume:
            return resume_campaign(arguments.resume)
        if arguments.config:
            return start_campaign(arguments)
        parser.print_help()
        return 2
    except configuration.ConfigError as error:
        print(f"Erreur : {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
