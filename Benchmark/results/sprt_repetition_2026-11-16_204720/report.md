# Rapport de benchmark : sprt_repetition

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +45 [+14 ; +77] |
| Rapport de vraisemblance (LLR) | 2,96 (bornes -2,94 et 2,94) |
| Parties comptées | 216 |
| Victoires | 64 (29,6 %) |
| Nulles | 116 (53,7 %) |
| Défaites | 36 (16,7 %) |
| Score | 56,5 % |
| NPS moyen | 682 924 |
| Temps moyen de réflexion | 180 ms |
| Profondeur moyenne | 5,7 |

Le test séquentiel (SPRT) accepte l'hypothèse d'un gain : la nouvelle version est meilleure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 20:47 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-new |
| Version du moteur | en cours (empreinte `b3359c8ae292`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | 1.0 (empreinte `80b2c622cfee`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 au maximum (arrêt du test séquentiel après 216) |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/sprt.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-v1.0 | 216 | 64 | 116 | 36 | 56,5 % | 62,5 / 108 | 59,5 / 108 | +45 [+14 ; +77] |

## Fins de partie

| Type | Parties |
|---|---:|
| nulle par répétition | 104 |
| arrêt par adjudication (gain) | 93 |
| mat | 7 |
| nulle par matériel insuffisant | 6 |
| arrêt par adjudication (nulle) | 4 |
| nulle par la règle des 50 coups | 2 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 10 093 |
| Coups tirés du livre | 0 |
| NPS moyen | 682 924 |
| Nœuds par coup | 61 870 |
| Profondeur moyenne | 5,7 |
| Temps moyen par coup | 180 ms |
| Occupation de la table de hachage | 9,4 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 4 495 976 |
| dont recherche de quiescence | 3 411 242 (75,9 %) |
| Coupures alpha-bêta | 170 219 |
| Coupures dès le premier coup essayé | 75 % |
| Positions retrouvées dans la table de transposition | 39 364 |
| Temps | 7 387 ms |
| NPS | 608 633 |

Version de référence sur les mêmes positions : 4 496 502 nœuds (-0,0 % pour la nouvelle version), 610 108 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition (celle-ci) | en cours | 1.0 | 216 | +45 | Amélioration significative |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
