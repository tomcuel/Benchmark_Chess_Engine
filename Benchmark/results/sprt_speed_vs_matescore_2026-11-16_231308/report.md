# Rapport de benchmark : sprt_speed_vs_matescore

## Résultat

| Métrique | Résultat |
|---|---:|
| Verdict | **Amélioration significative** |
| Écart Elo (nouvelle version − référence) | +132 [+95 ; +173] |
| Rapport de vraisemblance (LLR) | 2,95 (bornes -2,94 et 2,94) |
| Parties comptées | 209 |
| Victoires | 106 (50,7 %) |
| Nulles | 73 (34,9 %) |
| Défaites | 30 (14,4 %) |
| Score | 68,2 % |
| NPS moyen | 1 435 755 |
| Temps moyen de réflexion | 166 ms |
| Profondeur moyenne | 8,0 |

Le test séquentiel (SPRT) accepte l'hypothèse d'un gain : la nouvelle version est meilleure.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 23:13 |
| Mode | nouvelle version contre version de référence (SPRT) |
| Moteur | CleanedChess-speed |
| Version du moteur | speed (empreinte `022c44f30da0`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Version de référence | matescore (empreinte `8fb4b581a4c3`) |
| Options de la référence | Hash = 64, OwnBook = true, BookPlies = 10 |
| Hypothèses du SPRT | gain nul ou ≤ 0 Elo contre gain ≥ 10 Elo, risques 0.05 et 0.05 |
| Ouvertures | imposées aux deux versions (10 demi-coups tirés du livre), livre des moteurs coupé |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 400 au maximum (arrêt du test séquentiel après 209) |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/experiments/sprt_speed_vs_matescore.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CleanedChess-matescore | 209 | 106 | 73 | 30 | 68,2 % | 76,5 / 105 | 66,0 / 104 | +132 [+95 ; +173] |

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 132 |
| nulle par répétition | 44 |
| nulle par matériel insuffisant | 11 |
| nulle par la règle des 50 coups | 10 |
| arrêt par adjudication (nulle) | 7 |
| mat | 4 |
| pat | 1 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 12 884 |
| Coups tirés du livre | 0 |
| NPS moyen | 1 435 755 |
| Nœuds par coup | 123 139 |
| Profondeur moyenne | 8,0 |
| Temps moyen par coup | 166 ms |
| Occupation de la table de hachage | 23,3 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 402 790 |
| dont recherche de quiescence | 1 058 698 (75,5 %) |
| Coupures alpha-bêta | 60 590 |
| Coupures dès le premier coup essayé | 82 % |
| Positions retrouvées dans la table de transposition | 13 989 |
| Temps | 1 221 ms |
| NPS | 1 148 886 |

Version de référence sur les mêmes positions : 1 402 790 nœuds (+0,0 % pour la nouvelle version), 586 941 NPS.

## Comparaison avec les campagnes précédentes

| Date | Campagne | Nouvelle version | Référence | Parties | Écart Elo | Verdict |
|---|---|---|---|---:|---:|---|
| 2026-11-16 20:31 | sprt_test_handicap | en cours | 1.0 | 200 | +193 | Amélioration significative |
| 2026-11-16 20:47 | sprt_repetition | en cours | 1.0 | 216 | +45 | Amélioration significative |
| 2026-11-16 21:44 | sprt_hashmove_vs_v1.1 | hashmove | v1.1 | 274 | +116 | Amélioration significative |
| 2026-11-16 22:07 | sprt_ttage_vs_hashmove | ttage | hashmove | 364 | +65 | Amélioration significative |
| 2026-11-16 22:38 | sprt_matescore_vs_ttage | matescore | ttage | 400 | +1 | Résultat non concluant |
| 2026-11-16 23:13 | sprt_speed_vs_matescore (celle-ci) | speed | matescore | 209 | +132 | Amélioration significative |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
