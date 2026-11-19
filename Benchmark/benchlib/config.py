"""Chargement et validation de la configuration d'une campagne (fichier JSON)."""

import hashlib
import json
from pathlib import Path

BENCHMARK_DIR = Path(__file__).resolve().parent.parent

DEFAULTS = {
    "name": "campagne",
    "mode": "gauntlet",  # "gauntlet" : contre Stockfish bridé ; "sprt" : contre une autre version du moteur
    "fastchess": "tools/fastchess-src/fastchess",
    "time_control": "8+0.08",
    "concurrency": 4,
    "bench_depth": 6,
    "max_moves": 200,
    "draw_adjudication": {"movenumber": 40, "movecount": 8, "score": 10},
    "resign_adjudication": {"movecount": 4, "score": 700, "twosided": True},
    "engine": {
        "name": "CleanedChess",
        "version": "non précisée",
        "cmd": "../Chess_Engine/UCI/UCI.x",
        "book_file": "../Chess_Engine/Alpha_Beta/Data/Opening_in_Games.txt",
        "options": {"Hash": 64, "OwnBook": True, "BookPlies": 10},
    },
    "stockfish": {
        "cmd": "/opt/homebrew/bin/stockfish",
        "elo_levels": [1320, 1400, 1600, 1800, 2000],
        "options": {"Threads": 1, "Hash": 16},
    },
    # mode "sprt" uniquement
    "baseline": None,
    "sprt": {"elo0": 0, "elo1": 10, "alpha": 0.05, "beta": 0.05},
    "openings": {"plies": 10},
}


class ConfigError(Exception):
    """Erreur de configuration, avec un message lisible."""


def _merge(defaults, values):
    result = dict(defaults)
    for key, value in values.items():
        if isinstance(value, dict) and isinstance(result.get(key), dict):
            result[key] = _merge(result[key], value)
        else:
            result[key] = value
    return result


def resolve_path(path):
    """Chemin absolu : les chemins relatifs partent du dossier Benchmark/."""
    path = Path(path).expanduser()
    return path if path.is_absolute() else (BENCHMARK_DIR / path).resolve()


def file_sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1 << 16), b""):
            digest.update(block)
    return digest.hexdigest()


def _prepare_engine(engine, label):
    if not engine or "cmd" not in engine:
        raise ConfigError(f"« {label} » doit contenir au moins « cmd » (chemin du binaire UCI).")
    engine = _merge(DEFAULTS["engine"], engine)
    engine["cmd"] = str(resolve_path(engine["cmd"]))
    if not Path(engine["cmd"]).is_file():
        raise ConfigError(f"Binaire du moteur introuvable pour « {label} » : {engine['cmd']}\n"
                          "Compile-le avec : make -C Chess_Engine/UCI")
    if engine.get("book_file"):
        engine["book_file"] = str(resolve_path(engine["book_file"]))
        if not Path(engine["book_file"]).is_file():
            raise ConfigError(f"Livre d'ouvertures introuvable pour « {label} » : {engine['book_file']}")
    engine["sha256"] = file_sha256(engine["cmd"])
    return engine


def load_config(path, games=None):
    """Lit le fichier JSON, complète avec les valeurs par défaut et vérifie les chemins."""
    path = Path(path)
    if not path.is_file():
        raise ConfigError(f"Fichier de configuration introuvable : {path}")
    try:
        values = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        raise ConfigError(f"JSON invalide dans {path} : {error}") from error

    config = _merge(DEFAULTS, values)
    config["config_file"] = str(path.resolve())
    if config["mode"] not in ("gauntlet", "sprt"):
        raise ConfigError("« mode » doit valoir « gauntlet » ou « sprt ».")

    config["fastchess"] = str(resolve_path(config["fastchess"]))
    if not Path(config["fastchess"]).is_file():
        raise ConfigError(f"Fastchess introuvable : {config['fastchess']}")
    config["engine"] = _prepare_engine(values.get("engine", DEFAULTS["engine"]), "engine")

    if config["mode"] == "gauntlet":
        stockfish = config["stockfish"]
        if not Path(stockfish["cmd"]).is_file():
            raise ConfigError(f"Stockfish introuvable : {stockfish['cmd']}")
        levels = stockfish["elo_levels"]
        if not levels or any(not isinstance(level, int) for level in levels):
            raise ConfigError("« stockfish.elo_levels » doit être une liste d'entiers non vide.")
    else:
        config["baseline"] = _prepare_engine(values.get("baseline"), "baseline")
        if config["baseline"]["name"] == config["engine"]["name"]:
            config["baseline"]["name"] += "_baseline"

    if games is not None:
        config["games"] = games
    config.setdefault("games", 200)
    if config["games"] < 2:
        raise ConfigError("Il faut au moins 2 parties.")
    return config
