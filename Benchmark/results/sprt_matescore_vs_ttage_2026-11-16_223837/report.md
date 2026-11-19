# Rapport de benchmark : sprt_matescore_vs_ttage

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Résultat non concluant** |
| Écart Elo (nouvelle version − référence) | +1 [−27 ; +29] |
| Rapport de vraisemblance (LLR) | -0,55 (bornes -2,94 et 2,94) |
| Parties comptées | 400 |
| Victoires | 135 (33,8 %) |
| Nulles | 131 (32,8 %) |
| Défaites | 134 (33,5 %) |
| Score | 50,1 % |
| NPS moyen | 673 114 |
| Temps moyen de réflexion | 167 ms |
| Profondeur moyenne | 7,1 |

Le test séquentiel n'a atteint aucune de ses bornes et l'intervalle de confiance contient zéro : il faut plus de parties pour conclure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 22:38 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-matescore |
| Version du moteur | matescore (empreinte `8fb4b581a4c3`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | ttage (empreinte `35419ce43f43`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_matescore_vs_ttage.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-ttage | 400 | 135 | 131 | 134 | 50,1 % | 97,0 / 200 | 103,5 / 200 | +1 [−27 ; +29] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 258 |
| nulle par répétition | 66 |
| arrêt par adjudication (nulle) | 28 |
| nulle par matériel insuffisant | 19 |
| nulle par la règle des 50 coups | 16 |
| mat | 11 |
| pat | 2 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 24 343 |
| Coups tirés du livre | 0 |
| NPS moyen | 673 114 |
| Nœuds par coup | 58 236 |
| Profondeur moyenne | 7,1 |
| Temps moyen par coup | 167 ms |
| Occupation de la table de hachage | 12,2 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 402 790 |
| dont recherche de quiescence | 1 058 698 (75,5 %) |
| Coupures alpha-bêta | 60 590 |
| Coupures dès le premier coup essayé | 82 % |
| Positions retrouvées dans la table de transposition | 13 989 |
| Temps | 2 330 ms |
| NPS | 602 055 |

Version de référence sur les mêmes positions : 1 402 524 nœuds (+0,0 % pour la nouvelle version), 597 836 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove | ttage | hashmove | 364 | +65 | Amélioration significative |
| 2026-11-16 22:38 | sprt_matescore_vs_ttage (celle-ci) | matescore | ttage | 400 | +1 | Résultat non concluant |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
