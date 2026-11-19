# Rapport de benchmark : sprt_kingshield_vs_speed

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Résultat non concluant** |
| Écart Elo (nouvelle version − référence) | +23 [−7 ; +52] |
| Rapport de vraisemblance (LLR) | 0,76 (bornes -2,94 et 2,94) |
| Parties comptées | 400 |
| Victoires | 162 (40,5 %) |
| Nulles | 102 (25,5 %) |
| Défaites | 136 (34,0 %) |
| Score | 53,2 % |
| NPS moyen | 1 392 043 |
| Temps moyen de réflexion | 167 ms |
| Profondeur moyenne | 7,2 |

Le test séquentiel n'a atteint aucune de ses bornes et l'intervalle de confiance contient zéro : il faut plus de parties pour conclure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 23:31 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-kingshield |
| Version du moteur | kingshield (empreinte `d736af1e3be1`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | speed (empreinte `022c44f30da0`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_kingshield_vs_speed.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-speed | 400 | 162 | 102 | 136 | 53,2 % | 105,5 / 200 | 107,5 / 200 | +23 [−7 ; +52] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 294 |
| nulle par répétition | 43 |
| nulle par la règle des 50 coups | 24 |
| nulle par matériel insuffisant | 19 |
| arrêt par adjudication (nulle) | 15 |
| mat | 4 |
| pat | 1 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 23 984 |
| Coups tirés du livre | 0 |
| NPS moyen | 1 392 043 |
| Nœuds par coup | 125 019 |
| Profondeur moyenne | 7,2 |
| Temps moyen par coup | 167 ms |
| Occupation de la table de hachage | 23,0 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 289 101 |
| dont recherche de quiescence | 998 803 (77,5 %) |
| Coupures alpha-bêta | 51 055 |
| Coupures dès le premier coup essayé | 81 % |
| Positions retrouvées dans la table de transposition | 11 724 |
| Temps | 1 233 ms |
| NPS | 1 045 499 |

Version de référence sur les mêmes positions : 1 402 790 nœuds (-8,1 % pour la nouvelle version), 1 071 650 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove | ttage | hashmove | 364 | +65 | Amélioration significative |
| 2026-11-16 22:38 | sprt_matescore_vs_ttage | matescore | ttage | 400 | +1 | Résultat non concluant |
| 2026-11-16 23:13 | sprt_speed_vs_matescore | speed | matescore | 209 | +132 | Amélioration significative |
| 2026-11-16 23:31 | sprt_kingshield_vs_speed (celle-ci) | kingshield | speed | 400 | +23 | Résultat non concluant |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
