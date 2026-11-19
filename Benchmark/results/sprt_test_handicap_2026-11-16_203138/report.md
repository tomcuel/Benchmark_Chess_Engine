# Rapport de benchmark : sprt_test_handicap

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +193 [+160 ; +229] |
| Rapport de vraisemblance (LLR) | 2,84 (bornes -2,94 et 2,94) |
| Parties comptées | 200 |
| Victoires | 104 (52,0 %) |
| Nulles | 93 (46,5 %) |
| Défaites | 3 (1,5 %) |
| Score | 75,2 % |
| NPS moyen | 648 676 |
| Temps moyen de réflexion | 38 ms |
| Profondeur moyenne | 4,1 |

Le test séquentiel n'a pas atteint sa borne avec le nombre de parties prévu, mais l'intervalle de confiance à 95 % de l'écart Elo est entièrement positif.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 20:31 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-new |
| Version du moteur | en cours (empreinte `80b2c622cfee`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | 1.0 (empreinte `80b2c622cfee`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10, Move Overhead = 3000 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 2+0.02 (secondes + incrément par coup) |
| Parties prévues | 200 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_handicap.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-v1.0 | 200 | 104 | 93 | 3 | 75,2 % | 77,5 / 100 | 73,0 / 100 | +193 [+160 ; +229] |

## Fins de partie

| Type | Parties |
|---|---:|
| nulle par répétition | 92 |
| arrêt par adjudication (gain) | 85 |
| mat | 22 |
| arrêt par adjudication (nulle) | 1 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 6 565 |
| Coups tirés du livre | 0 |
| NPS moyen | 648 676 |
| Nœuds par coup | 11 826 |
| Profondeur moyenne | 4,1 |
| Temps moyen par coup | 38 ms |
| Occupation de la table de hachage | 1,1 % |

Mesures sur 10 positions fixes à profondeur 4 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 432 567 |
| dont recherche de quiescence | 382 520 (88,4 %) |
| Coupures alpha-bêta | 6 473 |
| Coupures dès le premier coup essayé | 68 % |
| Positions retrouvées dans la table de transposition | 1 032 |
| Temps | 808 ms |
| NPS | 535 355 |

Version de référence sur les mêmes positions : 432 567 nœuds (+0,0 % pour la nouvelle version), 522 423 NPS.

## Comparaison avec les campagnes précédentes

Aucune campagne précédente du même mode.

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
