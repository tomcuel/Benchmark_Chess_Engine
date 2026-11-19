# Rapport de benchmark : quick

## Résultat

| Métrique | Résultat |
|---|---:|
| Elo estimé (intervalle de confiance à 95 %) | 1849 ± 177 |
| Parties comptées | 20 |
| Victoires | 15 (75,0 %) |
| Nulles | 0 (0,0 %) |
| Défaites | 5 (25,0 %) |
| Score | 75,0 % |
| NPS moyen | 1 284 581 |
| Temps moyen de réflexion | 38 ms |
| Profondeur moyenne | 4,9 |

- ⚠️ Seulement 20 parties : l'estimation est indicative. Il en faut environ 200 pour une marge proche de ±50 Elo, et 1000 pour ±25.
- ⚠️ Score de 100 % contre SF1400 : ce palier ne renseigne que sur une borne, pas sur l'écart réel.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-17 02:22 |
| Mode | moteur contre Stockfish bridé |
| Moteur | CleanedChess |
| Version du moteur | 1.2 (empreinte `9faad60cda6c`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Adversaire | Stockfish avec `UCI_LimitStrength` (Threads = 1, Hash = 16) |
| Paliers | 1400, 1800 |
| Cadence | 2+0.02 (secondes + incrément par coup) |
| Parties prévues | 20 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/quick.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| SF1400 | 10 | 10 | 0 | 0 | 100,0 % | 5,0 / 5 | 5,0 / 5 | +∞ [+∞ ; +∞] |
| SF1800 | 10 | 5 | 0 | 5 | 50,0 % | 2,0 / 5 | 3,0 / 5 | +0 [−252 ; +252] |

Elo estimé avec les blancs : 1795 ± 231. Avec les noirs : 1908 ± 274.

L'Elo global est le classement qui rend la somme des scores attendus égale à la somme des scores obtenus, tous paliers confondus. Il dépend de la cadence, de la machine et de l'étalonnage de Stockfish bridé : ce n'est pas un classement officiel.

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 18 |
| mat | 2 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 747 |
| Coups tirés du livre | 30 |
| NPS moyen | 1 284 581 |
| Nœuds par coup | 26 782 |
| Profondeur moyenne | 4,9 |
| Temps moyen par coup | 38 ms |
| Occupation de la table de hachage | 2,4 % |

Mesures sur 10 positions fixes à profondeur 5 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 578 458 |
| dont recherche de quiescence | 493 657 (85,3 %) |
| Coupures alpha-bêta | 16 820 |
| Coupures dès le premier coup essayé | 79 % |
| Positions retrouvées dans la table de transposition | 3 090 |
| Temps | 567 ms |
| NPS | 1 020 208 |

## Comparaison avec les campagnes précédentes

| Date | Campagne | Version | Cadence | Parties | Elo estimé | NPS moyen | Nœuds du bench |
|---|---|---|---|---:|---:|---:|---:|
| 2026-11-16 20:03 | calibration | 1.0 | 8+0.08 | 118 | 1673 ± 89 | 660 152 | 4 496 502 |
| 2026-11-17 01:31 | calibration_v1.2 | 1.2 | 8+0.08 | 293 | 1983 ± 63 | 1 361 442 | 1 445 569 |
| 2026-11-17 02:22 | quick (celle-ci) | 1.2 | 2+0.02 | 20 | 1849 ± 177 | 1 284 581 | 578 458 |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
