# Rapport de benchmark : sprt_ttage_vs_hashmove

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +65 [+36 ; +94] |
| Rapport de vraisemblance (LLR) | 2,96 (bornes -2,94 et 2,94) |
| Parties comptées | 364 |
| Victoires | 150 (41,2 %) |
| Nulles | 131 (36,0 %) |
| Défaites | 83 (22,8 %) |
| Score | 59,2 % |
| NPS moyen | 670 007 |
| Temps moyen de réflexion | 170 ms |
| Profondeur moyenne | 7,0 |

Le test séquentiel (SPRT) accepte l'hypothèse d'un gain : la nouvelle version est meilleure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 22:07 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-ttage |
| Version du moteur | ttage (empreinte `35419ce43f43`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | hashmove (empreinte `9cc3304386b7`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 au maximum (arrêt du test séquentiel après 364) |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_ttage_vs_hashmove.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-hashmove | 364 | 150 | 131 | 83 | 59,2 % | 109,5 / 182 | 106,0 / 182 | +65 [+36 ; +94] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 228 |
| nulle par répétition | 68 |
| arrêt par adjudication (nulle) | 34 |
| nulle par matériel insuffisant | 22 |
| nulle par la règle des 50 coups | 7 |
| mat | 5 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 21 406 |
| Coups tirés du livre | 0 |
| NPS moyen | 670 007 |
| Nœuds par coup | 59 782 |
| Profondeur moyenne | 7,0 |
| Temps moyen par coup | 170 ms |
| Occupation de la table de hachage | 11,6 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 402 524 |
| dont recherche de quiescence | 1 058 698 (75,5 %) |
| Coupures alpha-bêta | 60 596 |
| Coupures dès le premier coup essayé | 82 % |
| Positions retrouvées dans la table de transposition | 13 989 |
| Temps | 2 403 ms |
| NPS | 583 655 |

Version de référence sur les mêmes positions : 1 509 910 nœuds (-7,1 % pour la nouvelle version), 603 481 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove (celle-ci) | ttage | hashmove | 364 | +65 | Amélioration significative |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
