# Rapport de benchmark : sprt_pstblend_vs_speed

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +44 [+19 ; +69] |
| Rapport de vraisemblance (LLR) | 2,40 (bornes -2,94 et 2,94) |
| Parties comptées | 600 |
| Victoires | 282 (47,0 %) |
| Nulles | 111 (18,5 %) |
| Défaites | 207 (34,5 %) |
| Score | 56,2 % |
| NPS moyen | 1 400 861 |
| Temps moyen de réflexion | 171 ms |
| Profondeur moyenne | 7,3 |

Le test séquentiel n'a pas atteint sa borne avec le nombre de parties prévu, mais l'intervalle de confiance à 95 % de l'écart Elo est entièrement positif.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-17 00:40 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-pstblend |
| Version du moteur | pstblend (empreinte `797aad4029df`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | speed (empreinte `022c44f30da0`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 600 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_pstblend_vs_speed.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-speed | 600 | 282 | 111 | 207 | 56,2 % | 172,0 / 300 | 165,5 / 300 | +44 [+19 ; +69] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 480 |
| nulle par répétition | 45 |
| nulle par la règle des 50 coups | 39 |
| nulle par matériel insuffisant | 22 |
| mat | 9 |
| arrêt par adjudication (nulle) | 5 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 34 494 |
| Coups tirés du livre | 0 |
| NPS moyen | 1 400 861 |
| Nœuds par coup | 127 730 |
| Profondeur moyenne | 7,3 |
| Temps moyen par coup | 171 ms |
| Occupation de la table de hachage | 21,4 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 445 569 |
| dont recherche de quiescence | 1 143 416 (79,1 %) |
| Coupures alpha-bêta | 54 861 |
| Coupures dès le premier coup essayé | 81 % |
| Positions retrouvées dans la table de transposition | 12 033 |
| Temps | 1 543 ms |
| NPS | 936 856 |

Version de référence sur les mêmes positions : 1 402 790 nœuds (+3,0 % pour la nouvelle version), 1 152 662 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove | ttage | hashmove | 364 | +65 | Amélioration significative |
| 2026-11-16 22:38 | sprt_matescore_vs_ttage | matescore | ttage | 400 | +1 | Résultat non concluant |
| 2026-11-16 23:13 | sprt_speed_vs_matescore | speed | matescore | 209 | +132 | Amélioration significative |
| 2026-11-16 23:31 | sprt_kingshield_vs_speed | kingshield | speed | 400 | +23 | Résultat non concluant |
| 2026-11-17 00:05 | sprt_pstblend_vs_kingshield | pstblend | kingshield | 400 | +15 | Résultat non concluant |
| 2026-11-17 00:40 | sprt_pstblend_vs_speed (celle-ci) | pstblend | speed | 600 | +44 | Amélioration significative |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
