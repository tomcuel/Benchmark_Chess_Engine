"""Lecture des parties écrites par Fastchess (résultats, fin de partie, statistiques de recherche)."""

import re

HEADER = re.compile(r'\[(\w+) "([^"]*)"\]')
# commentaire d'un coup : {+0.35/14 0.339s, n=128667, nps=379548, hashfull=31}
MOVE_COMMENT = re.compile(r"\{([+-]?(?:M?\d+(?:\.\d+)?))/(\d+) (\d+(?:\.\d+)?)s([^}]*)\}")
FIELD = re.compile(r"(\w+)=(-?\d+)")
MOVE_NUMBER = re.compile(r"^\d+\.(\.\.)?$")
RESULTS = {"1-0": (1.0, 0.0), "0-1": (0.0, 1.0), "1/2-1/2": (0.5, 0.5)}
# fins de partie qui ne relèvent pas du jeu lui-même
TECHNICAL_TERMINATIONS = ("time forfeit", "illegal move", "abandoned", "stalled connection", "unterminated")


def read_games(path):
    """Renvoie la liste des parties du fichier PGN (liste vide si le fichier n'existe pas)."""
    try:
        text = open(path, encoding="utf-8", errors="replace").read()
    except FileNotFoundError:
        return []
    games = []
    for block in re.split(r"(?=^\[Event )", text, flags=re.MULTILINE):
        if not block.startswith("[Event "):
            continue
        headers = dict(HEADER.findall(block))
        parts = block.split("\n\n", 1)
        body = parts[1] if len(parts) > 1 else ""
        result = headers.get("Result", "*")
        reasons = re.findall(r"\{([^{}]*)\}", body)
        reason = reasons[-1].split(",")[-1].strip() if reasons else ""
        games.append({
            "round": headers.get("Round", "?"),
            "white": headers.get("White", "?"),
            "black": headers.get("Black", "?"),
            "result": result,
            "termination": headers.get("Termination", "normal"),
            "reason": reason,
            "plies": int(headers.get("PlyCount", 0) or 0),
            "valid": result in RESULTS,
            "moves": _read_moves(body),
        })
    return games


def remove_replayed_games(games):
    """Ne garde que la dernière version de chaque partie.

    Quand une campagne est interrompue, Fastchess rejoue à la reprise les parties des paires qui
    n'étaient pas terminées : la même partie (même ronde, mêmes couleurs) apparaît alors deux fois
    dans le fichier. Renvoie (parties gardées, nombre de parties écartées).
    """
    last = {}
    for index, game in enumerate(games):
        last[(game["round"], game["white"], game["black"])] = index
    kept = [game for index, game in enumerate(games) if last[(game["round"], game["white"], game["black"])] == index]
    return kept, len(games) - len(kept)


def _read_moves(body):
    """Statistiques de recherche de chaque demi-coup, dans l'ordre (blancs, noirs, blancs, ...)."""
    moves = []
    for match in MOVE_COMMENT.finditer(body):
        fields = {name: int(value) for name, value in FIELD.findall(match.group(4))}
        moves.append({
            "depth": int(match.group(2)),
            "time": float(match.group(3)),
            "nodes": fields.get("n", 0),
            "nps": fields.get("nps", 0),
            "hashfull": fields.get("hashfull", 0),
        })
    return moves


def game_score(game, engine_name):
    """Score de la partie du point de vue du moteur (1, 0,5 ou 0), None si la partie n'a pas de résultat."""
    if not game["valid"]:
        return None
    white_score, black_score = RESULTS[game["result"]]
    return white_score if game["white"] == engine_name else black_score


def technical_loser(game):
    """Nom du joueur qui a perdu pour une raison technique (temps, coup illégal, plantage), sinon None."""
    if game["termination"] not in TECHNICAL_TERMINATIONS or game["result"] not in ("1-0", "0-1"):
        return None
    return game["black"] if game["result"] == "1-0" else game["white"]


def engine_search_statistics(games, engine_name):
    """Moyennes de la recherche du moteur sur ses coups réfléchis (les coups du livre sont exclus)."""
    depths, times, nps_values, nodes, hashfull, book_moves = [], [], [], [], [], 0
    for game in games:
        start = 0 if game["white"] == engine_name else 1
        for move in game["moves"][start::2]:
            if move["nodes"] == 0 and move["depth"] == 0:
                book_moves += 1
                continue
            depths.append(move["depth"])
            times.append(move["time"])
            nodes.append(move["nodes"])
            hashfull.append(move["hashfull"])
            if move["nps"] > 0 and move["time"] >= 0.01:
                nps_values.append(move["nps"])
    if not depths:
        return None
    average = lambda values: sum(values) / len(values) if values else 0.0
    return {
        "searched_moves": len(depths),
        "book_moves": book_moves,
        "average_depth": average(depths),
        "average_time_ms": average(times) * 1000.0,
        "average_nodes": average(nodes),
        "average_nps": average(nps_values),
        "average_hashfull": average(hashfull),
    }
