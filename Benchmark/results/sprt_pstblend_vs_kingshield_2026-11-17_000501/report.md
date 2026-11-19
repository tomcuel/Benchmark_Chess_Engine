# Rapport de benchmark : sprt_pstblend_vs_kingshield

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Résultat non concluant** |
| Écart Elo (nouvelle version − référence) | +15 [−14 ; +44] |
| Rapport de vraisemblance (LLR) | 0,49 (bornes -2,94 et 2,94) |
| Parties comptées | 400 |
| Victoires | 154 (38,5 %) |
| Nulles | 109 (27,3 %) |
| Défaites | 137 (34,2 %) |
| Score | 52,1 % |
| NPS moyen | 1 418 712 |
| Temps moyen de réflexion | 164 ms |
| Profondeur moyenne | 7,8 |

Le test séquentiel n'a atteint aucune de ses bornes et l'intervalle de confiance contient zéro : il faut plus de parties pour conclure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-17 00:05 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-pstblend |
| Version du moteur | pstblend (empreinte `797aad4029df`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | kingshield (empreinte `d736af1e3be1`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_pstblend_vs_kingshield.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-kingshield | 400 | 154 | 109 | 137 | 52,1 % | 105,0 / 200 | 103,5 / 200 | +15 [−14 ; +44] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 287 |
| nulle par répétition | 46 |
| nulle par la règle des 50 coups | 33 |
| nulle par matériel insuffisant | 20 |
| arrêt par adjudication (nulle) | 10 |
| mat | 4 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 25 791 |
| Coups tirés du livre | 0 |
| NPS moyen | 1 418 712 |
| Nœuds par coup | 123 483 |
| Profondeur moyenne | 7,8 |
| Temps moyen par coup | 164 ms |
| Occupation de la table de hachage | 23,1 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 445 569 |
| dont recherche de quiescence | 1 143 416 (79,1 %) |
| Coupures alpha-bêta | 54 861 |
| Coupures dès le premier coup essayé | 81 % |
| Positions retrouvées dans la table de transposition | 12 033 |
| Temps | 1 353 ms |
| NPS | 1 068 417 |

Version de référence sur les mêmes positions : 1 289 101 nœuds (+12,1 % pour la nouvelle version), 1 034 591 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove | ttage | hashmove | 364 | +65 | Amélioration significative |
| 2026-11-16 22:38 | sprt_matescore_vs_ttage | matescore | ttage | 400 | +1 | Résultat non concluant |
| 2026-11-16 23:13 | sprt_speed_vs_matescore | speed | matescore | 209 | +132 | Amélioration significative |
| 2026-11-16 23:31 | sprt_kingshield_vs_speed | kingshield | speed | 400 | +23 | Résultat non concluant |
| 2026-11-17 00:05 | sprt_pstblend_vs_kingshield (celle-ci) | pstblend | kingshield | 400 | +15 | Résultat non concluant |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
