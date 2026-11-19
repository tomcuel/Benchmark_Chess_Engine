"""Calcul des estimations Elo et de leur incertitude."""

import math

Z_95 = 1.96


def expected_score(rating, opponent_rating):
    """Score attendu : E_A = 1 / (1 + 10^((R_B - R_A) / 400))."""
    return 1.0 / (1.0 + 10.0 ** ((opponent_rating - rating) / 400.0))


def elo_difference(score):
    """Différence Elo correspondant à un score moyen : 400 × log10(S / (1 - S))."""
    if score <= 0.0:
        return -math.inf
    if score >= 1.0:
        return math.inf
    return 400.0 * math.log10(score / (1.0 - score))


def score_statistics(wins, draws, losses):
    """Score moyen et son erreur type, calculée sur les trois issues possibles (1, 0,5, 0)."""
    games = wins + draws + losses
    if games == 0:
        return None
    score = (wins + 0.5 * draws) / games
    variance = (wins * (1 - score) ** 2 + draws * (0.5 - score) ** 2 + losses * (0 - score) ** 2) / games
    return {"games": games, "score": score, "stderr": math.sqrt(variance / games)}


def elo_from_results(wins, draws, losses):
    """Différence Elo contre un adversaire, avec intervalle de confiance à 95 %.

    Les bornes valent ±inf quand le score (ou sa borne) touche 0 % ou 100 % :
    dans ce cas la formule ne peut plus donner qu'une borne d'un seul côté.
    """
    stats = score_statistics(wins, draws, losses)
    if stats is None:
        return None
    score, stderr = stats["score"], stats["stderr"]
    low = max(0.0, score - Z_95 * stderr)
    high = min(1.0, score + Z_95 * stderr)
    stats.update({"elo": elo_difference(score), "elo_low": elo_difference(low), "elo_high": elo_difference(high)})
    return stats


def estimate_rating(results):
    """Elo du moteur à partir de parties contre des adversaires de classements connus.

    results : liste de (classement de l'adversaire, score de la partie 1 / 0,5 / 0).
    On cherche le classement R tel que la somme des scores attendus soit égale à la
    somme des scores obtenus (maximum de vraisemblance du modèle Elo). L'intervalle
    de confiance vient de la dispersion des résultats autour des scores attendus.
    """
    if not results:
        return None
    total = sum(score for _, score in results)
    games = len(results)
    estimate = {"games": games, "score": total / games}
    if total <= 0.0 or total >= games:
        # tout perdu ou tout gagné : le modèle ne donne qu'une borne
        ratings = [rating for rating, _ in results]
        estimate.update({"elo": None, "bound": "max" if total <= 0.0 else "min",
                         "bound_value": min(ratings) if total <= 0.0 else max(ratings)})
        return estimate

    low, high = -2000.0, 6000.0
    for _ in range(80):
        middle = (low + high) / 2.0
        if sum(expected_score(middle, rating) for rating, _ in results) < total:
            low = middle
        else:
            high = middle
    rating = (low + high) / 2.0

    spread = sum((score - expected_score(rating, opponent)) ** 2 for opponent, score in results)
    slope = sum(expected_score(rating, opponent) * (1 - expected_score(rating, opponent))
                for opponent, _ in results) * math.log(10) / 400.0
    margin = Z_95 * math.sqrt(spread) / slope if slope > 0 else math.inf
    estimate.update({"elo": rating, "margin": margin})
    return estimate


def reliability_warnings(total_games, per_opponent):
    """Avertissements quand les données ne permettent pas une estimation fiable."""
    warnings = []
    if total_games < 100:
        warnings.append(f"Seulement {total_games} parties : l'estimation est indicative. "
                        "Il en faut environ 200 pour une marge proche de ±50 Elo, et 1000 pour ±25.")
    for name, stats in per_opponent.items():
        if stats["games"] and stats["score"] in (0.0, 1.0):
            warnings.append(f"Score de {stats['score'] * 100:.0f} % contre {name} : ce palier ne renseigne que sur "
                            "une borne, pas sur l'écart réel.")
    scores = [(stats["rating"], stats["score"]) for stats in per_opponent.values()
              if stats.get("rating") is not None and stats["games"]]
    scores.sort()
    for (rating_a, score_a), (rating_b, score_b) in zip(scores, scores[1:]):
        if score_b > score_a + 0.10:
            warnings.append(f"Résultats non monotones : meilleur score contre {rating_b} que contre {rating_a}. "
                            "L'échelle de Stockfish bridé est peu fiable à cadence rapide, ou l'échantillon est trop petit.")
            break
    return warnings
