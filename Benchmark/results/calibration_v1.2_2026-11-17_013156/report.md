# Rapport de benchmark : calibration_v1.2

## Résultat

| Métrique | Résultat |
|---|---:|
| Elo estimé (intervalle de confiance à 95 %) | 1983 ± 63 |
| Parties comptées | 293 |
| Victoires | 207 (70,6 %) |
| Nulles | 6 (2,0 %) |
| Défaites | 80 (27,3 %) |
| Score | 71,7 % |
| NPS moyen | 1 361 442 |
| Temps moyen de réflexion | 187 ms |
| Profondeur moyenne | 6,8 |

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-17 01:31 |
| Mode | moteur contre Stockfish bridé |
| Moteur | CleanedChess |
| Version du moteur | 1.2 (empreinte `6bdda7815054`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Adversaire | Stockfish avec `UCI_LimitStrength` (Threads = 1, Hash = 16) |
| Paliers | 1320, 1400, 1600, 1800, 2000, 2200 |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 300 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/calibration.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| SF1320 | 49 | 48 | 0 | 1 | 98,0 % | 24,0 / 24 | 24,0 / 25 | +672 [+478 ; +∞] |
| SF1400 | 49 | 46 | 1 | 2 | 94,9 % | 24,0 / 24 | 22,5 / 25 | +508 [+364 ; +∞] |
| SF1600 | 49 | 42 | 0 | 7 | 85,7 % | 22,0 / 24 | 20,0 / 25 | +311 [+199 ; +531] |
| SF1800 | 50 | 30 | 1 | 19 | 61,0 % | 17,5 / 25 | 13,0 / 25 | +78 [−17 ; +185] |
| SF2000 | 48 | 22 | 1 | 25 | 46,9 % | 11,5 / 24 | 11,0 / 24 | −22 [−124 ; +77] |
| SF2200 | 48 | 19 | 3 | 26 | 42,7 % | 8,0 / 24 | 12,5 / 24 | −51 [−154 ; +44] |

Elo estimé avec les blancs : 2016 ± 80. Avec les noirs : 1951 ± 96.

L'Elo global est le classement qui rend la somme des scores attendus égale à la somme des scores obtenus, tous paliers confondus. Il dépend de la cadence, de la machine et de l'étalonnage de Stockfish bridé : ce n'est pas un classement officiel.

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 238 |
| mat | 49 |
| perte au temps | 7 |
| nulle par répétition | 4 |
| nulle par la règle des 50 coups | 2 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

7 partie(s) écartée(s) des résultats, perdue(s) par l'adversaire pour une raison technique :
- SF2000 : Black loses on time (86ms overrun)
- SF2000 : White loses on time (81ms overrun)
- SF1400 : Black loses on time (36ms overrun)
- SF1600 : Black loses on time (95ms overrun)
- SF1320 : Black loses on time (73ms overrun)
- SF2200 : White loses on time (24ms overrun)
- SF2200 : Black loses on time (4ms overrun)

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 12 462 |
| Coups tirés du livre | 604 |
| NPS moyen | 1 361 442 |
| Nœuds par coup | 139 940 |
| Profondeur moyenne | 6,8 |
| Temps moyen par coup | 187 ms |
| Occupation de la table de hachage | 17,5 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 1 445 569 |
| dont recherche de quiescence | 1 143 416 (79,1 %) |
| Coupures alpha-bêta | 54 861 |
| Coupures dès le premier coup essayé | 81 % |
| Positions retrouvées dans la table de transposition | 12 033 |
| Temps | 1 711 ms |
| NPS | 844 867 |

## Comparaison avec les campagnes précédentes

| Date | Campagne | Version | Cadence | Parties | Elo estimé | NPS moyen | Nœuds du bench |
|---|---|---|---|---:|---:|---:|---:|
| 2026-11-16 20:03 | calibration | 1.0 | 8+0.08 | 118 | 1673 ± 89 | 660 152 | 4 496 502 |
| 2026-11-17 01:31 | calibration_v1.2 (celle-ci) | 1.2 | 8+0.08 | 293 | 1983 ± 63 | 1 361 442 | 1 445 569 |

Par rapport à « calibration » (2026-11-16 20:03, même cadence) : +310 Elo, marge combinée ± 109 (écart significatif). Pour comparer deux versions, le mode SPRT est plus précis.

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
