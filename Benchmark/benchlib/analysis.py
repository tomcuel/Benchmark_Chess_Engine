"""Analyse d'une campagne : résultats, estimation Elo, parties invalides, statistiques de recherche."""

import collections
from pathlib import Path

from . import pgn, rating, tournament


def _record():
    return {"wins": 0, "draws": 0, "losses": 0, "white_points": 0.0, "white_games": 0,
            "black_points": 0.0, "black_games": 0, "plies": 0}


def analyse(config, campaign_dir):
    """Lit les parties de la campagne et renvoie un résumé (dictionnaire sérialisable en JSON)."""
    engine_name = config["engine"]["name"]
    plan = tournament.plan_games(config)
    games, replayed_games = pgn.remove_replayed_games(pgn.read_games(Path(campaign_dir) / tournament.PGN_FILE))

    if config["mode"] == "gauntlet":
        ratings = {tournament.stockfish_name(level): level for level in config["stockfish"]["elo_levels"]}
    else:
        ratings = {config["baseline"]["name"]: None}

    records = collections.defaultdict(_record)
    terminations = collections.Counter()
    engine_technical_losses, opponent_technical_losses, unfinished = [], [], 0
    rated_results, white_results, black_results = [], [], []

    for game in games:
        engine_is_white = game["white"] == engine_name
        opponent = game["black"] if engine_is_white else game["white"]
        score = pgn.game_score(game, engine_name)
        if score is None:
            unfinished += 1
            continue
        terminations[_termination_label(game)] += 1

        loser = pgn.technical_loser(game)
        if loser == engine_name:
            # défaut du moteur testé : la défaite compte, et elle est signalée
            engine_technical_losses.append({"opponent": opponent, "reason": game["reason"]})
        elif loser is not None:
            # défaut de l'adversaire : la partie n'est pas comptée dans les résultats
            opponent_technical_losses.append({"opponent": opponent, "reason": game["reason"]})
            continue

        record = records[opponent]
        record["wins" if score == 1.0 else "draws" if score == 0.5 else "losses"] += 1
        side = "white" if engine_is_white else "black"
        record[f"{side}_points"] += score
        record[f"{side}_games"] += 1
        record["plies"] += game["plies"]
        if ratings.get(opponent) is not None:
            rated_results.append((ratings[opponent], score))
            (white_results if engine_is_white else black_results).append((ratings[opponent], score))

    opponents = {}
    for name in ratings:
        record = records.get(name, _record())
        stats = rating.elo_from_results(record["wins"], record["draws"], record["losses"]) or {"games": 0, "score": 0.0}
        stats.update(record)
        stats["rating"] = ratings[name]
        stats["average_plies"] = record["plies"] / stats["games"] if stats["games"] else 0
        opponents[name] = stats

    counted = sum(stats["games"] for stats in opponents.values())
    wins = sum(stats["wins"] for stats in opponents.values())
    draws = sum(stats["draws"] for stats in opponents.values())
    losses = sum(stats["losses"] for stats in opponents.values())

    summary = {
        "planned_games": plan["total"],
        "played_games": len(games) - unfinished,
        "counted_games": counted,
        "complete": len(games) - unfinished >= plan["total"],
        "wins": wins, "draws": draws, "losses": losses,
        "score": (wins + 0.5 * draws) / counted if counted else 0.0,
        "opponents": opponents,
        "terminations": dict(terminations.most_common()),
        "engine_technical_losses": engine_technical_losses,
        "opponent_technical_losses": opponent_technical_losses,
        "unfinished_games": unfinished,
        "replayed_games": replayed_games,
        "search": pgn.engine_search_statistics(games, engine_name),
    }

    if config["mode"] == "gauntlet":
        summary["rating"] = rating.estimate_rating(rated_results)
        summary["rating_white"] = rating.estimate_rating(white_results)
        summary["rating_black"] = rating.estimate_rating(black_results)
        summary["warnings"] = rating.reliability_warnings(counted, opponents)
    else:
        baseline = opponents[config["baseline"]["name"]]
        summary["elo_difference"] = {key: baseline.get(key) for key in ("elo", "elo_low", "elo_high")}
        summary["sprt"] = sprt = tournament.read_sprt_result(campaign_dir)
        # le test séquentiel arrête le tournoi dès qu'il atteint une de ses bornes : la campagne est alors terminée
        summary["stopped_by_sprt"] = bool(sprt and (sprt["llr"] >= sprt["upper"] or sprt["llr"] <= sprt["lower"]))
        summary["complete"] = summary["complete"] or summary["stopped_by_sprt"]
        summary["verdict"] = sprt_verdict(summary["sprt"], baseline, summary["complete"])
        summary["warnings"] = []
    return summary


def _termination_label(game):
    reason = game["reason"].lower()
    if game["termination"] in pgn.TECHNICAL_TERMINATIONS:
        return {"time forfeit": "perte au temps", "illegal move": "coup illégal"}.get(game["termination"], "abandon ou plantage")
    if "mates" in reason:
        return "mat"
    if "adjudication" in reason:
        return "arrêt par adjudication (gain)" if game["result"] != "1/2-1/2" else "arrêt par adjudication (nulle)"
    if "repetition" in reason:
        return "nulle par répétition"
    if "fifty" in reason:
        return "nulle par la règle des 50 coups"
    if "stalemate" in reason:
        return "pat"
    if "insufficient" in reason:
        return "nulle par matériel insuffisant"
    return game["reason"] or game["termination"]


def sprt_verdict(sprt, results, complete):
    """Les trois issues possibles d'une comparaison de versions.

    Le test séquentiel (SPRT) tranche dès qu'il atteint une de ses bornes. S'il ne l'a pas fait quand toutes
    les parties prévues sont jouées, l'intervalle de confiance à 95 % de l'écart Elo décide : entièrement
    positif, entièrement négatif, ou à cheval sur zéro (non concluant). Une campagne interrompue avant
    une borne du SPRT reste non concluante, pour ne pas conclure sur un arrêt choisi en cours de route.
    """
    elo_low, elo_high = results.get("elo_low"), results.get("elo_high")
    if sprt and sprt["llr"] >= sprt["upper"]:
        return {"label": "Amélioration significative",
                "detail": "Le test séquentiel (SPRT) accepte l'hypothèse d'un gain : la nouvelle version est meilleure."}
    if sprt and sprt["llr"] <= sprt["lower"]:
        if elo_high is not None and elo_high < 0:
            return {"label": "Régression significative",
                    "detail": "Le test séquentiel rejette l'hypothèse d'un gain, et l'intervalle de confiance à 95 % "
                              "de l'écart Elo est entièrement négatif."}
        return {"label": "Résultat non concluant",
                "detail": "Le test séquentiel rejette l'hypothèse d'un gain, sans que la régression soit établie : "
                          "la nouvelle version n'est pas meilleure de façon mesurable."}
    if not complete:
        return {"label": "Résultat non concluant",
                "detail": "Campagne interrompue avant que le test séquentiel n'atteigne une de ses bornes."}
    if elo_low is not None and elo_low > 0:
        return {"label": "Amélioration significative",
                "detail": "Le test séquentiel n'a pas atteint sa borne avec le nombre de parties prévu, mais "
                          "l'intervalle de confiance à 95 % de l'écart Elo est entièrement positif."}
    if elo_high is not None and elo_high < 0:
        return {"label": "Régression significative",
                "detail": "L'intervalle de confiance à 95 % de l'écart Elo est entièrement négatif."}
    return {"label": "Résultat non concluant",
            "detail": "Le test séquentiel n'a atteint aucune de ses bornes et l'intervalle de confiance contient zéro : "
                      "il faut plus de parties pour conclure."}
