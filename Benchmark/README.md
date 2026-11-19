## Elo benchmark of the chess engine

This folder measures the level of the engine. It makes it play against Stockfish limited in strength at several levels, estimates its Elo with a confidence interval, measures the performances of its search and writes a Markdown report. It also compares two versions of the engine to tell if a change made it better.

The games are run by [Fastchess](https://github.com/Disservin/fastchess), the Python code prepares the tournament, follows it, reads the games and does the statistics. The engine is used through its [UCI program](../Chess_Engine/UCI/).

#### Table of contents
* [Path tree](#path-tree)
* [Requirements](#requirements)
* [Usage](#usage-from-the-benchmark-folder)
* [What a campaign does](#what-a-campaign-does)
* [Is the engine reliable ?](#is-the-engine-reliable-)
* [Measuring the Elo : gauntlet mode](#measuring-the-elo--gauntlet-mode)
* [Comparing two versions : sprt mode](#comparing-two-versions--sprt-mode)
* [The games that don't count](#the-games-that-dont-count)
* [Performances of the search](#performances-of-the-search)
* [Results](#results)
* [Configuration](#configuration-json-file)
* [Files of a campaign](#files-of-a-campaign)
* [Limitations](#limitations)


## Path tree
```
Benchmark/
├── benchmark.py # the command to launch
├── run_sprt.sh # compiles the engine, checks it, keeps it under a name and compares it to a reference
│
├── benchlib/ # the modules of the benchmark
│   ├── config # reading the JSON configuration, default values, checking the paths, fingerprint of the engine
│   ├── tournament # the Fastchess command, following the games, interrupting and resuming, openings of the sprt mode
│   ├── pgn # reading the games : result, ending, and depth, time, nodes of each move
│   ├── rating # Elo computation and confidence intervals, warnings when the data are too weak
│   ├── enginebench # performances of the search with the bench command of the engine
│   ├── analysis # summary of a campaign : results per opponent, invalid games, SPRT verdict
│   └── report # Markdown report and history of the campaigns
│
├── configs/ # the configurations of the campaigns
├── engines/ # the versions of the engine kept as references (not in the repository : made with --snapshot)
├── results/ # one folder per campaign, and history.jsonl for the comparisons
└── tools/ # Fastchess, a git submodule compiled from its sources
```


## Requirements
* The UCI engine compiled : `make -C Chess_Engine/UCI`
* Stockfish : `brew install stockfish` _(expected in `/opt/homebrew/bin/stockfish`, the path can be changed in the configuration)_
* Fastchess : a git submodule fixed on the version `v1.8.3-alpha`. `git clone --recursive` fetches it with the project, or `git submodule update --init` after a simple clone, then `cd Benchmark/tools/fastchess-src && make -j`
* The reference version of the sprt mode : `python3 benchmark.py --snapshot v1.2`, from the `Benchmark` folder, once the engine is compiled. The programs of `engines/` aren't in the repository, they are programs compiled for macOS on Apple Silicon
* Python 3, without any additional package


## Usage _(from the Benchmark folder)_
| Goal | Command |
|---|---|
| Estimating the Elo against Stockfish | `python3 benchmark.py --config configs/calibration.json --games 300` |
| Quick try, 20 games in one minute | `python3 benchmark.py --config configs/quick.json` |
| Resuming an interrupted campaign, or writing its report again | `python3 benchmark.py --resume results/<folder>` |
| Keeping the current engine as a reference | `python3 benchmark.py --snapshot v1.2` |
| Comparing the current engine to the reference | `python3 benchmark.py --config configs/sprt.json --games 400` |
| The whole comparison in one command | `./run_sprt.sh <new name> <reference name> [games] [time control]` |

Other options of `benchmark.py` : `--name` _(name of the campaign)_, `--concurrency` _(games at the same time)_, `--engine` _(the program copied by `--snapshot`)_.

The texts shown in the terminal and the reports are written in French.

#### Keeping a version
`--snapshot <name>` copies `Chess_Engine/UCI/UCI.x` into `engines/<name>/`. This folder isn't a git repository, so a version is identified by its name and by the fingerprint of its program _(the first 12 characters of its SHA-256, written in every report)_. A name already used is refused : a reference never changes.

#### Trying a change of the engine
`./run_sprt.sh hashmove v1.1 400` does, in this order :
1. Compiling the engine
2. Running the perft, to check the move generation
3. Running the bench, to see if the number of nodes changed
4. Keeping the engine as `engines/hashmove/` if it doesn't exist yet
5. Writing `configs/experiments/sprt_hashmove_vs_v1.1.json`
6. Running the comparison of `hashmove` against `v1.1`


## What a campaign does
`python3 benchmark.py --config <file>` :

1. **Loads the configuration**, completes it with the default values, checks that the engine, Stockfish, Fastchess and the opening book exist, and computes the fingerprint of the engine
2. **Plans the games** : an even number against each opponent, so each one is met as many times with white as with black. 200 games asked against 6 levels become 204 _(34 against each)_
3. **Runs the bench** of the engine
4. **Builds the Fastchess command** and saves it with the configuration in `campaign.json`, before the first game
5. **Runs the tournament** and shows the progress every 10 games
6. **Reads the games**, counts the results, puts the invalid games apart
7. **Computes** the Elo _(or the difference and the verdict in sprt mode)_ and the averages of the search
8. **Writes the report** and adds one line to `results/history.jsonl`, which the next reports use to compare

#### Interrupting and resuming
* Ctrl+C stops Fastchess cleanly : it saves its state in `fastchess_state.json` after every game
* The report is written anyway with the games played, and marked as incomplete
* `--resume results/<folder>` continues where it stopped. Fastchess plays again the pairs of games that weren't finished, so the same game can be twice in the PGN file : only the last one of each _(same round, same colors)_ is kept
* On a campaign already finished, `--resume` only writes the report again, for example after a change in the report code


## Is the engine reliable ?
A crash or an illegal move loses the game, and a wrong rule changes the level measured. So the engine is checked before any game :

* **Perft** : the move generation is compared to the known values on 7 reference positions
* **Sanitizers** : games of the engine against itself, compiled with the address and undefined behavior sanitizers
* **Bench** : the search of 10 fixed positions at a fixed depth. Its number of nodes only depends on the code of the engine. If it changes between two versions, the search has changed. If it doesn't, a difference of speed is only a difference of speed

During the campaigns, every loss of the engine on time, by illegal move or by crash is counted and written in the report. The version 1.2 has none in 293 games against Stockfish.


## Measuring the Elo : gauntlet mode

#### The opponent
* Stockfish with `UCI_LimitStrength = true`, one thread and 16 MB of hash, at several values of `UCI_Elo`. 1320 is its lowest level
* One Stockfish per level, named `SF1320`, `SF1400`, ... : the engine meets all of them, they don't play each other _(a gauntlet)_
* **Several levels are needed.** A score of 98 % against one opponent only says « much stronger », the difference can be 500 or 900. The levels close to the engine, where it scores between 30 % and 70 %, are the ones that give the information

#### The conditions
| Parameter | Value |
|---|---|
| Time control | 8 s + 0.08 s per move, the same for everybody |
| Games at the same time | 4 _(the number of performance cores of this machine)_ |
| Colors | each pair of games is played with the colors reversed |
| Openings | from the starting position, the opening book of the engine is active on its first 10 half-moves |
| Hash of the engine | 64 MB |
| Longest game | 200 moves, then it's a draw |

* **The opening book stays on** in this mode : it gives a direction to the opening and makes the games different from each other. The number of half-moves is limited to 10 so the search is measured, not the book
* **Adjudication** : a game already decided is stopped before its end, to play more games in the same time
    * win : both engines see a score beyond 7 pawns for the same side during 4 moves
    * draw : after the move 40, both engines see a score under 0.10 pawn during 8 moves

#### The Elo
The Elo model gives the score expected against an opponent : `E = 1 / (1 + 10^((R_opponent - R) / 400))`. At equal ratings it's 50 %, with 200 points more it's 76 %, with 400 points more it's 91 %.

* **Estimated Elo** : the rating `R` for which the sum of the expected scores against all the opponents met is equal to the sum of the scores obtained. It's found by bisection. All the games are used together, each level with its own rating
* **Per opponent** : the report also gives the difference with each level alone, with `400 × log10(S / (1 - S))` where `S` is the score against it
* **With white and with black** : the same estimation on the games of each color, to see if one side is weaker

#### The uncertainty
A result is always given with a 95 % confidence interval, `1983 ± 63` means « between 1920 and 2046, 19 times out of 20 if the measure was done again ».

* For one opponent, the interval comes from the dispersion of the results _(1, 0.5 or 0)_ around the average score. The draws reduce it : two draws say more than a win and a loss
* For the global Elo, it comes from the dispersion of the results around the expected scores, divided by the sensitivity of the score to the rating
* **It only gets smaller with the square root of the number of games** : about ±90 for 120 games, ±60 for 300, ±25 for 1500. Four times more games for half of the margin
* A score of 0 % or 100 % gives an interval open on one side, written `+∞`

#### Warnings
The report says when the data don't allow a reliable estimation :
* fewer than 100 games
* 0 % or 100 % against a level, which then only gives a bound
* a better score against a stronger level than against a weaker one _(more than 10 points)_ : the scale of Stockfish limited in strength isn't reliable at this time control, or the sample is too small


## Comparing two versions : sprt mode
The difference between two Elo estimations is too vague to judge a change : with ±60 on each, the difference is known at ±85. Making the two versions play each other is much more accurate.

#### The conditions
* The version tested plays against a reference kept in `engines/`, with the same time control and the same hash
* **Same openings for both.** `openings.pgn` is made from the opening book of the engine : its 2594 different starts of 10 half-moves. Each one is played twice with the colors reversed, so an opening that favors one side favors each version once
* **The own book of the engines is turned off** : this mode measures the search and the evaluation, not the book
* The openings are drawn at random with a fixed seed, so a campaign can be played again in the same order

#### The sequential test
A fixed number of games wastes time : a change worth +100 Elo is obvious after 100 games, a change worth +5 isn't after 1000. The SPRT _(Sequential Probability Ratio Test)_ decides after each game if it knows enough.

* Two hypotheses : `elo0 = 0` _(the new version isn't better)_ and `elo1 = 10` _(it's better by 10 Elo)_
* After each game, the **LLR** _(log-likelihood ratio)_ says which hypothesis explains the results better. It starts at 0, goes up with the good results and down with the bad ones
* With `alpha = beta = 0.05` _(5 % of wrong conclusions accepted on each side)_, the bounds are −2.94 and +2.94
* **LLR ≥ +2.94** : the gain is accepted, the tournament stops
* **LLR ≤ −2.94** : the gain is rejected, the tournament stops
* Otherwise the games go on until the number asked

#### The verdict
| Situation | Verdict |
|---|---|
| The LLR reached its upper bound | significant improvement |
| The LLR reached its lower bound, and the whole interval of the Elo difference is negative | significant regression |
| The LLR reached its lower bound, but the interval contains 0 | not conclusive _(not better in a measurable way)_ |
| All the games played without reaching a bound, the whole interval is positive | significant improvement |
| All the games played without reaching a bound, the whole interval is negative | significant regression |
| All the games played, the interval contains 0 | not conclusive |
| Campaign interrupted before a bound | not conclusive |

An interrupted campaign is never conclusive, whatever its score : choosing when to stop by looking at the results is the best way to conclude wrongly.


## The games that don't count
| Case | Treatment |
|---|---|
| The engine loses on time, by illegal move or by crash | counted as a loss, and listed in the report |
| The opponent loses for one of these reasons | removed from the results, and listed in the report |
| Game without result _(interrupted)_ | ignored |
| Game played twice after a resume | only the last one is kept |

A technical loss of the opponent says nothing about the level of the engine, it's removed. A technical loss of the engine is its own fault, it's a real loss. In the campaign of the version 1.2, Stockfish lost 7 games on time by a few tens of milliseconds, with 4 games running at the same time.


## Performances of the search
Two sources, both in the report :

#### During the games
Fastchess saves in the PGN what the engine said for each move _(depth, time, nodes, nodes per second, filling of the hash)_. The averages are made on the moves really searched, the moves of the opening book are counted apart.

| Indicator | Version 1.2 |
|---|---:|
| Moves searched | 12 462 |
| Moves from the book | 604 |
| Nodes per second | 1 361 442 |
| Nodes per move | 139 940 |
| Depth | 6.8 |
| Time per move | 187 ms |
| Filling of the hash table | 17.5 % |

#### With the bench
The `bench` command of the engine, on 10 fixed positions at depth 6. These counters don't exist in the games.

| Indicator | Version 1.0 | Version 1.2 |
|---|---:|---:|
| Nodes | 4 496 502 | 1 445 569 |
| Share of quiescence nodes | | 79.1 % |
| Beta cutoffs | | 54 861 |
| Cutoffs made by the first move tried | | 81 % |
| Hits in the transposition table | | 12 033 |


## Results

#### Elo of the version 1.2
Campaign `calibration_v1.2` of 2026-11-17, 300 games at 8 s + 0.08 s, Apple Silicon, 4 games at the same time.

| Metric | Result |
|---|---:|
| Estimated Elo (95 % confidence interval) | 1983 ± 63 |
| Games counted | 293 |
| Wins | 207 (70.6 %) |
| Draws | 6 (2.0 %) |
| Losses | 80 (27.3 %) |
| Technical losses of the engine | 0 |

| Opponent | Games | W | D | L | Score | Elo difference (95 % interval) |
|---|---:|---:|---:|---:|---:|---:|
| SF1320 | 49 | 48 | 0 | 1 | 98.0 % | +672 [+478 ; +∞] |
| SF1400 | 49 | 46 | 1 | 2 | 94.9 % | +508 [+364 ; +∞] |
| SF1600 | 49 | 42 | 0 | 7 | 85.7 % | +311 [+199 ; +531] |
| SF1800 | 50 | 30 | 1 | 19 | 61.0 % | +78 [−17 ; +185] |
| SF2000 | 48 | 22 | 1 | 25 | 46.9 % | −22 [−124 ; +77] |
| SF2200 | 48 | 19 | 3 | 26 | 42.7 % | −51 [−154 ; +44] |

* Estimated Elo with the white pieces : 2016 ± 80, with the black pieces : 1951 ± 96
* Endings : 238 adjudications, 49 mates, 4 draws by repetition, 2 draws by the 50 moves rule
* The three levels from 1800 to 2200 are the ones that place the engine : the first three are almost all won

#### Comparison with the version 1.0
| Campaign | Version | Games | Estimated Elo | Average NPS | Nodes of the bench |
|---|---|---:|---:|---:|---:|
| calibration | 1.0 | 118 | 1673 ± 89 | 660 152 | 4 496 502 |
| calibration_v1.2 | 1.2 | 293 | 1983 ± 63 | 1 361 442 | 1 445 569 |

The difference is +310 Elo with a combined margin of ±109. The engine searches three times fewer nodes for the same depth, and twice as many nodes per second.

#### SPRT of each change
Each version contains the previous ones, and played against the previous one.

| New version | Reference | Change | Games | W / D / L | Elo difference | Verdict |
|---|---|---|---:|---:|---:|---|
| v1.1 | v1.0 | draw detection in the search | 216 | | +45 [+14 ; +77] | improvement |
| hashmove | v1.1 | move of the transposition table first | 274 | 144 / 74 / 56 | +116 [+80 ; +153] | improvement |
| ttage | hashmove | ageing of the transposition table | 364 | 150 / 131 / 83 | +65 [+36 ; +94] | improvement |
| matescore | ttage | mate scores with the distance | 400 | 135 / 131 / 134 | +1 [−27 ; +29] | not conclusive |
| speed | matescore | speed of the search | 209 | 106 / 73 / 30 | +132 [+95 ; +173] | improvement |
| kingshield | speed | king safety corrected | 400 | 162 / 102 / 136 | +23 [−7 ; +52] | not conclusive |
| pstblend | kingshield | middle game / endgame blend corrected | 400 | 154 / 109 / 137 | +15 [−14 ; +44] | not conclusive |
| pstblend | speed | the two evaluation corrections together | 600 | 282 / 111 / 207 | +44 [+19 ; +69] | improvement |

* The big gains stopped the test early : 209 games for +132, 274 for +116
* `matescore` was kept as a correction : nothing shows a regression
* `kingshield` and `pstblend` weren't conclusive alone, so they were tested together on 600 games. The LLR ended at 2.40 without reaching 2.94 : the verdict comes from the interval, entirely positive
* The version 1.2 is `pstblend` with its version number changed

#### How to read these numbers
* The Elo depends on the time control, on the machine and on the calibration of Stockfish limited in strength : it's used to follow the engine over time, it isn't an official rating
* The differences between two close versions of the same engine are bigger than against another opponent : the SPRT gains sum to about +400, the measure against Stockfish gives +310
* Each report ends with the last campaigns of the same mode. In gauntlet mode it gives the difference with the last one played at the same time control, and says if it's bigger than the two margins combined


## Configuration _(JSON file)_
A field left out takes its default value, so a configuration can be short.

| Field | Default | Role |
|---|---|---|
| `name` | `campagne` | name of the campaign, and start of the name of its folder |
| `mode` | `gauntlet` | `gauntlet` or `sprt` |
| `games` | 200 | total number of games, replaced by `--games` |
| `time_control` | `8+0.08` | `seconds+increment`, the same for everybody |
| `concurrency` | 4 | games played at the same time |
| `bench_depth` | 6 | depth of the bench |
| `max_moves` | 200 | a longer game is a draw |
| `draw_adjudication` | `movenumber` 40, `movecount` 8, `score` 10 | early draw, `null` to turn it off |
| `resign_adjudication` | `movecount` 4, `score` 700, `twosided` true | early win, `null` to turn it off |
| `fastchess` | `tools/fastchess-src/fastchess` | the Fastchess program |

| Field | Role |
|---|---|
| `engine` | the engine tested : `cmd`, `name`, `version` _(free label)_, `book_file` and its UCI `options` _(`Hash`, `OwnBook`, `BookPlies`, `Seed`, `Move Overhead`)_ |
| `stockfish` | gauntlet mode : `cmd`, `elo_levels` and `options` |
| `baseline` | sprt mode : the reference version, same fields as `engine` |
| `sprt` | sprt mode : `elo0`, `elo1`, `alpha`, `beta` |
| `openings` | sprt mode : `plies`, the number of half-moves imposed |

The relative paths start from the `Benchmark` folder.

#### Configurations given
| File | Content |
|---|---|
| `calibration.json` | 6 levels from 1320 to 2200, the one used for the results above |
| `baseline.json` | 5 levels from 1320 to 2000, 200 games |
| `quick.json` | 20 games at 2 s + 0.02 s, to check that everything works |
| `sprt.json` | the current engine against `engines/v1.2` |
| `experiments/` | the configuration of each SPRT already played |

Testing another setting of the engine doesn't need any change in the code : a copy of a configuration with other `options` _(a hash of 16 MB, a book of 4 half-moves, ...)_ is a new campaign.


## Files of a campaign
`results/<name>_<date>_<time>/` :

| File | Content |
|---|---|
| `report.md` | the report |
| `games.pgn` | all the games, with the depth, the time, the nodes and the speed of each move |
| `summary.json` | all the numbers of the report |
| `campaign.json` | the exact configuration, the fingerprint of the engine, the bench and the Fastchess command |
| `fastchess_output.txt` | what Fastchess wrote during the tournament |
| `fastchess_state.json` | the state of the tournament, used to resume |
| `openings.pgn` | sprt mode : the openings imposed |

A report has these parts : the result, the campaign _(date, version, configuration, opponents)_, the results per opponent with each color, the endings of the games and the invalid ones, the performances of the search, the comparison with the previous campaigns.

`results/history.jsonl` has one line per campaign : date, name, version and fingerprint of the engine, time control, games, Elo and margin, average speed and depth, nodes of the bench.


## Limitations
* **The scale is the one of Stockfish limited in strength**, which plays weak moves on purpose and doesn't play like a human of the same rating. Almost no game is drawn _(2 %)_
* **One time control and one machine.** The level at 8 s + 0.08 s isn't the level at 5 minutes, and another machine gives another speed
* The counters of cutoffs and of the transposition table come from the bench, not from the games
* The sprt mode doesn't measure the opening book
* The machine mustn't be loaded during a campaign, otherwise the engines lose think time and games on time
* 300 games take 24 minutes and give ±63 : going to ±25 would need about 1500 games, two hours
