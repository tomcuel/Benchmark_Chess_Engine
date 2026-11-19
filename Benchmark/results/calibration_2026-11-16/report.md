# Rapport de benchmark : calibration

## Résultat

| Métrique | Résultat |
|---|---:|
| Elo estimé (intervalle de confiance à 95 %) | 1673 ± 89 |
| Parties comptées | 118 |
| Victoires | 52 (44,1 %) |
| Nulles | 8 (6,8 %) |
| Défaites | 58 (49,2 %) |
| Score | 47,5 % |
| NPS moyen | 660 152 |
| Temps moyen de réflexion | 184 ms |
| Profondeur moyenne | 5,2 |

- ⚠️ Résultats non monotones : meilleur score contre 1400 que contre 1320. L'échelle de Stockfish bridé est peu fiable à cadence rapide, ou l'échantillon est trop petit.

## Campagne

| Paramètre | Valeur |
|---|---|
| Date | 2026-11-16 20:03 |
| Mode | moteur contre Stockfish bridé |
| Moteur | CleanedChess |
| Version du moteur | 1.0 (empreinte `80b2c622cfee`) |
| Options du moteur | Hash = 64, OwnBook = true, BookPlies = 10 |
| Adversaire | Stockfish avec `UCI_LimitStrength` (Threads = 1, Hash = 16) |
| Paliers | 1320, 1400, 1600, 1800, 2000, 2200 |
| Cadence | 8+0.08 (secondes + incrément par coup) |
| Parties prévues | 120 |
| Parties en parallèle | 4 |
| Fichier de configuration | `Benchmark/configs/calibration.json` |

## Résultats par adversaire

| Adversaire | Parties | V | N | D | Score | Avec les blancs | Avec les noirs | Écart Elo (IC 95 %) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| SF1320 | 20 | 14 | 1 | 5 | 72,5 % | 8,5 / 10 | 6,0 / 10 | +168 [+25 ; +412] |
| SF1400 | 20 | 18 | 2 | 0 | 95,0 % | 10,0 / 10 | 9,0 / 10 | +512 [+353 ; +∞] |
| SF1600 | 20 | 7 | 3 | 10 | 42,5 % | 5,0 / 10 | 3,5 / 10 | −53 [−214 ; +88] |
| SF1800 | 19 | 7 | 1 | 11 | 39,5 % | 4,0 / 10 | 3,5 / 9 | −74 [−262 ; +77] |
| SF2000 | 20 | 4 | 0 | 16 | 20,0 % | 3,0 / 10 | 1,0 / 10 | −241 [−639 ; −89] |
| SF2200 | 19 | 2 | 1 | 16 | 13,2 % | 2,0 / 10 | 0,5 / 9 | −328 [−∞ ; −169] |

Elo estimé avec les blancs : 1761 ± 128. Avec les noirs : 1584 ± 121.

L'Elo global est le classement qui rend la somme des scores attendus égale à la somme des scores obtenus, tous paliers confondus. Il dépend de la cadence, de la machine et de l'étalonnage de Stockfish bridé : ce n'est pas un classement officiel.

## Fins de partie

| Type | Parties |
|---|---:|
| arrêt par adjudication (gain) | 88 |
| mat | 22 |
| nulle par répétition | 6 |
| perte au temps | 2 |
| nulle par la règle des 50 coups | 1 |
| pat | 1 |

Aucune défaite technique du moteur testé (ni perte au temps, ni coup illégal, ni plantage).

2 partie(s) écartée(s) des résultats, perdue(s) par l'adversaire pour une raison technique :
- SF2200 : White loses on time (6ms overrun)
- SF1800 : White loses on time (31ms overrun)

## Performances de la recherche

Mesures relevées pendant les parties, sur les coups réfléchis (hors livre d'ouvertures) :

| Indicateur | Valeur |
|---|---:|
| Coups réfléchis | 5 406 |
| Coups tirés du livre | 257 |
| NPS moyen | 660 152 |
| Nœuds par coup | 62 395 |
| Profondeur moyenne | 5,2 |
| Temps moyen par coup | 184 ms |
| Occupation de la table de hachage | 8,4 % |

Mesures sur 10 positions fixes à profondeur 6 (commande `bench`). Le nombre de nœuds ne dépend que du code du moteur, pas de la machine :

| Indicateur | Valeur |
|---|---:|
| Nœuds explorés | 4 496 502 |
| dont recherche de quiescence | 3 411 676 (75,9 %) |
| Coupures alpha-bêta | 170 216 |
| Coupures dès le premier coup essayé | 75 % |
| Positions retrouvées dans la table de transposition | 39 502 |
| Temps | 7 498 ms |
| NPS | 599 693 |

## Comparaison avec les campagnes précédentes

| Date | Campagne | Version | Cadence | Parties | Elo estimé | NPS moyen | Nœuds du bench |
|---|---|---|---|---:|---:|---:|---:|
| 2026-11-16 20:28 | quick | 1.0 | 2+0.02 | 20 | 1600 ± 154 | 739 034 | 1 315 063 |
| 2026-11-16 20:29 | interrupt_test | 1.0 | 2+0.02 | 20 | 1600 ± 154 | 672 926 | 1 315 063 |
| 2026-11-16 20:30 | interrupt_test2 | 1.0 | 2+0.02 | 40 | 1528 ± 113 | 680 348 | 1 315 063 |
| 2026-11-16 20:03 | calibration (celle-ci) | 1.0 | 8+0.08 | 118 | 1673 ± 89 | 660 152 | 4 496 502 |

## Fichiers

- `games.pgn` : toutes les parties, avec profondeur, temps, nœuds et NPS de chaque coup
- `summary.json` : les chiffres de ce rapport
- `campaign.json` : la configuration exacte de la campagne
- `fastchess_output.txt` : la sortie de Fastchess
