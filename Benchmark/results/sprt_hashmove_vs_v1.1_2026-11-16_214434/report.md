# Rapport de benchmark : sprt_hashmove_vs_v1.1

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +116 [+80 ; +153] |
| Rapport de vraisemblance (LLR) | 2,95 (bornes -2,94 et 2,94) |
| Parties comptées | 274 |
| Victoires | 144 (52,6 %) |
| Nulles | 74 (27,0 %) |
| Défaites | 56 (20,4 %) |
| Score | 66,1 % |
| NPS moyen | 668 208 |
| Temps moyen de réflexion | 171 ms |
| Profondeur moyenne | 6,8 |

Le test séquentiel (SPRT) accepte l'hypothèse d'un gain : la nouvelle version est meilleure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 21:44 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-hashmove |
| Version du moteur | hashmove (empreinte `9cc3304386b7`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | v1.1 (empreinte `520563175880`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 au maximum (arrêt du test séquentiel après 274) |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_hashmove_vs_v1.1.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-v1.1 | 274 | 144 | 74 | 56 | 66,1 % | 98,0 / 137 | 83,0 / 137 | +116 [+80 ; +153] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 189 |
| nulle par répétition | 37 |
| nulle par matériel insuffisant | 13 |
| nulle par la règle des 50 coups | 13 |
| arrêt par adjudication (nulle) | 11 |
| mat | 11 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 15 683 |
| Coups tirés du livre | 0 |
| NPS moyen | 668 208 |
| Nœuds par coup | 59 516 |
| Profondeur moyenne | 6,8 |
| Temps moyen par coup | 171 ms |
| Occupation de la table de hachage | 11,3 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 509 910 |
| dont recherche de quiescence | 1 130 404 (74,9 %) |
| Coupures alpha-bêta | 64 379 |
| Coupures dès le premier coup essayé | 81 % |
| Positions retrouvées dans la table de transposition | 16 278 |
| Temps | 2 557 ms |
| NPS | 590 500 |

Version de référence sur les mêmes positions : 4 495 976 nœuds (-66,4 % pour la nouvelle version), 601 790 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 (celle-ci) | hashmove | v1.1 | 274 | +116 | Amélioration significative |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
