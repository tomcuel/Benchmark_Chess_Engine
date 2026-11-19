"""Lancement des tournois avec Fastchess (contre Stockfish bridé, ou version contre version avec SPRT)."""

import re
import signal
import subprocess
from pathlib import Path

STATE_FILE = "fastchess_state.json"
OUTPUT_FILE = "fastchess_output.txt"
PGN_FILE = "games.pgn"
OPENINGS_FILE = "openings.pgn"
ANSI = re.compile(r"\x1b\[[0-9;]*m")
LLR_LINE = re.compile(r"LLR:\s*(-?\d+(?:\.\d+)?)\s*\(.*?\)\s*\((-?\d+(?:\.\d+)?),\s*(-?\d+(?:\.\d+)?)\)")


def _option_value(value):
    if isinstance(value, bool):
        return "true" if value else "false"
    return str(value)


def _engine_arguments(name, cmd, options):
    arguments = ["-engine", f"cmd={cmd}", f"name={name}"]
    arguments += [f"option.{key}={_option_value(value)}" for key, value in options.items()]
    return arguments


def _own_engine_arguments(engine, use_own_book=None):
    options = dict(engine.get("options", {}))
    if use_own_book is not None:
        options["OwnBook"] = use_own_book
    if options.get("OwnBook", True) and engine.get("book_file"):
        # le chemin du livre est donné explicitement, pour qu'un binaire copié ailleurs le retrouve
        options["BookFile"] = engine["book_file"]
    return _engine_arguments(engine["name"], engine["cmd"], options)


def stockfish_name(level):
    return f"SF{level}"


def plan_games(config):
    """Nombre de parties réellement jouées : un nombre pair par adversaire (couleurs alternées)."""
    if config["mode"] == "gauntlet":
        levels = len(config["stockfish"]["elo_levels"])
        rounds = max(1, round(config["games"] / levels / 2))
        return {"rounds": rounds, "games_per_opponent": rounds * 2, "total": rounds * 2 * levels}
    rounds = max(1, config["games"] // 2)
    return {"rounds": rounds, "games_per_opponent": rounds * 2, "total": rounds * 2}


def write_openings(book_file, destination, plies):
    """Fabrique un fichier PGN d'ouvertures à partir du livre du moteur (une ligne = une partie).

    Sert au mode version contre version : les deux moteurs jouent les mêmes ouvertures, couleurs inversées.
    """
    lines = set()
    for line in Path(book_file).read_text(encoding="utf-8").splitlines():
        moves = line.split()[:plies]
        if len(moves) == plies:
            lines.add(" ".join(moves))
    games = []
    for line in sorted(lines):
        moves = line.split()
        text = " ".join(f"{index // 2 + 1}. {move}" if index % 2 == 0 else move for index, move in enumerate(moves))
        games.append(f'[Event "Opening"]\n[Result "*"]\n\n{text} *\n')
    Path(destination).write_text("\n".join(games), encoding="utf-8")
    return len(games)


def build_command(config, campaign_dir):
    """Ligne de commande Fastchess de la campagne."""
    plan = plan_games(config)
    command = [config["fastchess"]]
    if config["mode"] == "gauntlet":
        command += _own_engine_arguments(config["engine"])
        stockfish = config["stockfish"]
        for level in stockfish["elo_levels"]:
            options = dict(stockfish.get("options", {}))
            options.update({"UCI_LimitStrength": True, "UCI_Elo": level})
            command += _engine_arguments(stockfish_name(level), stockfish["cmd"], options)
        command += ["-tournament", "gauntlet", "-seeds", "1"]
    else:
        # mêmes ouvertures imposées aux deux versions : leur propre livre est coupé
        command += _own_engine_arguments(config["engine"], use_own_book=False)
        command += _own_engine_arguments(config["baseline"], use_own_book=False)
        plies = config["openings"]["plies"]
        count = write_openings(config["engine"]["book_file"], Path(campaign_dir) / OPENINGS_FILE, plies)
        if count == 0:
            raise RuntimeError("Aucune ouverture n'a pu être tirée du livre du moteur.")
        command += ["-openings", f"file={OPENINGS_FILE}", "format=pgn", "order=random", f"plies={plies}"]
        command += ["-srand", str(config.get("seed", 20261116))]
        sprt = config["sprt"]
        command += ["-sprt", f"elo0={sprt['elo0']}", f"elo1={sprt['elo1']}", f"alpha={sprt['alpha']}",
                    f"beta={sprt['beta']}", "model=logistic"]

    command += ["-each", f"tc={config['time_control']}"]
    command += ["-rounds", str(plan["rounds"]), "-games", "2", "-concurrency", str(config["concurrency"])]
    command += ["-maxmoves", str(config["max_moves"])]
    draw, resign = config["draw_adjudication"], config["resign_adjudication"]
    if draw:
        command += ["-draw", f"movenumber={draw['movenumber']}", f"movecount={draw['movecount']}", f"score={draw['score']}"]
    if resign:
        command += ["-resign", f"movecount={resign['movecount']}", f"score={resign['score']}",
                    f"twosided={_option_value(resign.get('twosided', True))}"]
    command += ["-recover", "-ratinginterval", "0", "-autosaveinterval", "1", "-config", f"outname={STATE_FILE}"]
    command += ["-pgnout", f"file={PGN_FILE}", "nodes=true", "nps=true", "hashfull=true"]
    return command


def resume_command(config):
    return [config["fastchess"], "-config", f"file={STATE_FILE}"]


def run(command, campaign_dir, total_games, already_played=0, progress=print):
    """Lance Fastchess et suit l'avancement. Renvoie (code de sortie, interrompu)."""
    campaign_dir = Path(campaign_dir)
    finished = already_played
    interrupted = False
    with open(campaign_dir / OUTPUT_FILE, "a", encoding="utf-8") as output:
        process = subprocess.Popen(command, cwd=campaign_dir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, errors="replace")
        try:
            for line in process.stdout:
                line = ANSI.sub("", line)
                # les avertissements sur le PV de Stockfish bridé (bestmove différent du PV) sont normaux et très bavards
                if line.startswith(("Info;", "Position;", "Moves;")) or "does not match beginning of last PV" in line:
                    continue
                output.write(line)
                if line.startswith("Finished game"):
                    finished += 1
                    if finished % 10 == 0 or finished == total_games:
                        progress(f"  {finished}/{total_games} parties terminées")
                elif line.startswith("SPRT") or "LLR:" in line:
                    progress("  " + line.strip())
        except KeyboardInterrupt:
            interrupted = True
            progress("\nInterruption demandée : arrêt de Fastchess et sauvegarde de l'état...")
            process.send_signal(signal.SIGINT)
        try:
            return_code = process.wait(timeout=60)
        except subprocess.TimeoutExpired:
            process.kill()
            return_code = process.wait()
    return return_code, interrupted


def read_sprt_result(campaign_dir):
    """Dernier rapport de vraisemblance (LLR) écrit par Fastchess, avec ses bornes."""
    try:
        text = (Path(campaign_dir) / OUTPUT_FILE).read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return None
    matches = LLR_LINE.findall(text)
    if not matches:
        return None
    llr, lower, upper = (float(value) for value in matches[-1])
    return {"llr": llr, "lower": lower, "upper": upper}
