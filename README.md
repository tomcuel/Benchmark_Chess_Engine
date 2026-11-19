# Chess Engine in C
> A chess engine written in C (negamax with alpha-beta pruning, transposition table, opening book), with the UCI protocol and an automated benchmark that measures its level. The current version, 1.2, is estimated at **1983 ± 63 Elo** against Stockfish limited in strength (8 s + 0.08 s per move), and in real conditions it beats Stockfish limited at **1900 Elo** in a 2 minutes game.

The goal of this repository is to benchmark my original chess engine, the one of [Upgraded_C_Chess_Engine](https://github.com/tomcuel/Upgraded_C_Chess_Engine) : a chess game with a graphical interface made with SDL, with its own AI, whose level had never been measured. This version keeps only the engine : it was separated from the interface, given the UCI protocol to play against other engines, checked and corrected, then measured with an automated benchmark. The graphical game was removed.

Most of this work _(the benchmark, the corrections, the documentation)_ was done by vibe coding with Claude Code, to save time : the goal was to evaluate the engine, not to learn how to build a benchmark.

| | |
|---|---|
| Estimated level | 1983 ± 63 Elo _(293 short games against Stockfish limited from 1320 to 2200)_ |
| Speed | 1 360 000 positions per second, depth 6.8 on average in 187 ms |
| Reliability | 0 loss on time, illegal move or crash in 293 games |
| Language | C for the engine _(standard library only)_, Python 3 for the benchmark _(no package)_ |

#### Table of contents
* [Path tree](#path-tree)
* [To run this project](#to-run-this-project)
* [How the engine is organized](#how-the-engine-is-organized)
* [What the engine does](#what-the-engine-does)
* [What was done in this version](#what-was-done-in-this-version)
* [Corrections made to the engine](#corrections-made-to-the-engine)
* [Improvements validated one by one](#improvements-validated-one-by-one)
* [How the level is measured](#how-the-level-is-measured)
* [Results](#results)
* [Limitations](#limitations)
* [Structure of the code](#structure-of-the-code)


## Path tree
```
Cleaned_Chess_Engine/
├── Chess_Engine/
│   ├── Alpha_Beta/       # the search, the evaluation, the move ordering, the transposition table, the opening book
│   ├── Game_Treatment/   # the board, the move generation, the log of the game, the FEN positions
│   ├── UCI/              # the programs : the UCI engine, the perft check, the self-play check
│   └── Useful/           # the types, the constants and the functions unrelated to chess
│
└── Benchmark/            # the tournaments against Stockfish, the Elo estimation, the SPRT tests, the reports
    ├── benchlib/         # the Python modules
    ├── configs/          # the configurations of the campaigns
    ├── engines/          # the versions of the engine kept as references
    ├── results/          # one folder per campaign played, with its report
    └── tools/            # Fastchess, as a git submodule
```

Each folder has its own README, which goes into the details of the code :

| Folder | What its README explains |
|---|---|
| [Game_Treatment](./Chess_Engine/Game_Treatment/) | how a position and a move are stored, the log of the game, the attacks of the pieces, the generation of the legal moves, making and undoing a move |
| [Alpha_Beta](./Chess_Engine/Alpha_Beta/) | the search step by step, the quiescence search, the draw detection, the transposition table, the move ordering, every term of the evaluation, the opening book |
| [UCI](./Chess_Engine/UCI/) | the UCI protocol with a full exchange, the commands and the options, the time given to a move, the perft and how to find a wrong move with it |
| [Useful](./Chess_Engine/Useful/) | the types and the constants shared by the engine, the clock, the random numbers |
| [Benchmark](./Benchmark/) | how to run a campaign, the method of the Elo estimation and of the SPRT, all the results, the configuration |


## To run this project
In the root of the project folder :

| Goal | Command |
|---|---|
| Compiling the three programs | `make -C Chess_Engine` |
| Removing the objects and the programs | `make -C Chess_Engine realclean` |
| Talking to the engine | `./Chess_Engine/UCI/UCI.x` |
| Checking the move generation | `./Chess_Engine/UCI/Perft.x` |
| Measuring the search on 10 fixed positions | `./Chess_Engine/UCI/UCI.x bench 6` |
| Watching the engine play against itself | `./Chess_Engine/UCI/Engine_Check.x 1000 16 42` |

#### Asking a move to the engine
```
> setoption name OwnBook value false
> position startpos moves e2e4 e7e5
> go depth 5
info depth 1 score cp 51 nodes 59 nps 59000 hashfull 0 time 0 pv b1c3
...
info depth 5 score cp 27 nodes 36755 nps 1113787 hashfull 2 time 33 pv b1c3
bestmove b1c3
```
_(the lines starting with `>` are typed, without the `>`. Without the first line, the move comes from the opening book)_

#### Playing against it
The engine has no board of its own : a chess interface shows the board, holds the clocks and asks the engine for its moves. Any interface that speaks UCI works, here with [BanksiaGUI](https://banksiagui.com/), free and available on macOS :

1. Installing it : `brew install --cask banksiagui`
2. Compiling the engine : `make -C Chess_Engine`
3. Opening BanksiaGUI. On the first launch macOS may block it because it doesn't come from the App Store : right click on the application in `/Applications`, then **Open**
4. Adding the engine, once :
    * opening the settings with **⌘ ,** _(the menu **Settings**, that macOS can move to the menu **banksiagui**)_
    * choosing the page **Engines** in the list on the left
    * clicking **Add** _(« Add a new engine »)_ : the window **Add/Edit Engine** opens
    * in **Engine file:**, choosing the file `Chess_Engine/UCI/UCI.x` of this project, or dropping it on the window
    * leaving the name, the protocol and the **Working folder:** empty : BanksiaGUI detects them by asking the engine
    * validating : the engine appears in the list **Engines (n)**, and **Configure** shows its options
5. Starting a game against it :
    * **New game** _(⌘ /)_ opens the window **Game setup**
    * **White:** and **Black:** : **Human** for the color you play, the engine for the other one
    * **Time control:** : a game with a clock, for example 5 minutes + 3 seconds per move
    * **Create a new game on the current tab**, then validating
    * **New Human Game** is a game between two humans, it doesn't use the engine

The names above are the ones of the interface of BanksiaGUI 0.58 in English. They were read from the texts of the program, not on the screen : the place of a button can differ a little.

A few things to know :
* **A time control with a clock** is the best choice : the engine shares its time over the game. With a fixed time per move, it thinks exactly this time at every move
* **Its first 10 half-moves** come from its opening book and are drawn at random, so it doesn't always play the same opening
* **The level measured (about 1980)** was at 8 s + 0.08 s per move : with more time it searches deeper, it's at least as strong
* **The option `Hash`** _(64 MB)_ can be changed in the settings, page **Engines**, button **Configure**. The other options of the engine are described in [UCI](./Chess_Engine/UCI/#options)
* The engine finds its opening book by itself, from wherever the interface launches it
* **BanksiaGUI has its own opening books** _(Cerebellum and Fruit)_, active by default on the first 20 half-moves for the engines : in a game between two engines, these moves are played at once by the interface, so the game seems to start in the middle game. They can be turned off or shortened in the settings, page of the books : the column **Active** of each book, and **max ply:** for the number of half-moves. With them turned off, this engine uses its own book again
* Other free interfaces work the same way : Cute Chess, En Croissant, Arena _(Windows)_

#### Playing against Stockfish at a chosen level
BanksiaGUI comes with Stockfish 15.1, which plays at full strength by default _(around 3500 Elo or more, far above any human and above this engine)_. Its level can be lowered, to play against it or to make it play this engine :

1. Opening the settings with **⌘ ,**, page **Engines**
2. Selecting **stockfish15.1** in the list, then **Configure**
3. Setting the options of Stockfish :

| Option | Value | Effect |
|---|---|---|
| `UCI_LimitStrength` | checked | turns the limitation on |
| `UCI_Elo` | from 1350 to 2850 | the level aimed at, in Elo |
| `Skill Level` | from 0 to 20 | another way to weaken it, coarser : it doesn't match a precise Elo. Only used when `UCI_LimitStrength` is off |

4. Validating, then starting a game with **New game** as above, with Stockfish for one color

Choosing a level :
* **For a level close to this engine** : `UCI_LimitStrength` checked and `UCI_Elo` at **2000**, or, without the limitation, `Skill Level` at **6 or 7**. It's the level where the engine scored about 50 % in the benchmark _(46.9 % against 2000, 42.7 % against 2200)_
* `UCI_Elo` and `Skill Level` are the same mechanism : Stockfish turns the Elo asked into a skill level, with `Elo ≈ 1346.6 + 143.4 × level^0.806` _(formula of the source code of Stockfish 15 and 16, not measured here)_. That gives about 1350 for the level 0, 1955 for 6, 2035 for 7, 2470 for 10. The two only differ because `UCI_Elo` allows the values between two levels
* **With a slower time control**, the engine gets relatively stronger : Stockfish limited chooses its move at a depth fixed by the level, so more time barely helps it, while this engine searches deeper. At 2 minutes + 1 second, a `UCI_Elo` of 2100 or 2200 is probably closer to an even game than 2000
* The benchmark used Stockfish 19, whose `UCI_Elo` goes from 1320 to 3190 : the scales of two versions of Stockfish aren't exactly the same, so a level of 2000 in 15.1 is close to the 2000 of the benchmark, not identical
* The `UCI_Elo` of Stockfish was calibrated against other engines at a fast time control : it isn't the rating of a human of the same level, a human at 2000 often finds it stronger or weaker depending on the kind of position
* To measure this engine against Stockfish at a precise level and time control, the benchmark does it : `elo_levels` and `time_control` in a configuration of [Benchmark](./Benchmark/#configuration-json-file), for example `"time_control": "120+1"` for 2 minutes + 1 second

#### Measuring its level
From the `Benchmark` folder : `python3 benchmark.py --config configs/calibration.json --games 300` _(24 minutes)_. It needs Python 3, Stockfish and Fastchess, see [Benchmark](./Benchmark/).

#### Requirements
* The engine : a C compiler and `make`, nothing else
* Playing against it : a chess interface that speaks UCI, for example BanksiaGUI _(`brew install --cask banksiagui`)_
* The benchmark : Python 3 without any package, Stockfish _(`brew install stockfish`)_, and Fastchess : a git submodule in `Benchmark/tools/fastchess-src`, fetched with `git clone --recursive` _(or `git submodule update --init` after a simple clone)_ then compiled with `make -C Benchmark/tools/fastchess-src -j`
* Everything was built and measured on macOS with Apple Silicon


## How the engine is organized
```
        a chess interface, Fastchess, or a terminal
                          │  text : "position ...", "go ..."      ▲  "bestmove e2e4"
                          ▼                                       │
                 ┌─────────────────────────────────────────────────────┐
   UCI           │  reads the commands, rebuilds the position,         │
                 │  chooses the time, runs the search in a thread      │
                 └─────────────────────────────────────────────────────┘
                          │                                       ▲
                          ▼                                       │
                 ┌─────────────────────────────────────────────────────┐
   Alpha_Beta    │  opening book  →  otherwise : iterative deepening   │
                 │  search (alpha-beta) → quiescence → evaluation      │
                 │  helped by the transposition table and the ordering │
                 └─────────────────────────────────────────────────────┘
                          │  generate the moves, make, undo, is it check ?
                          ▼
                 ┌─────────────────────────────────────────────────────┐
   Game_Treatment│  bitboards, log of the game, legal moves            │
                 └─────────────────────────────────────────────────────┘
                          │
                          ▼
   Useful           types, constants, clock, random numbers
```

* Each layer only uses the ones under it. `Alpha_Beta` and `Game_Treatment` have no `main` : they are a library, the programs are in `UCI`
* The state of the game is kept in global variables _(the bitboards, the log, the color to play)_ : the search makes a move, calls itself, and undoes the move on the same board, without ever copying it
* For one move asked, the engine looks at about 140 000 positions in the benchmark games. Each of them goes through the generation of the moves, which is why this part and the small functions it calls weigh so much in the speed


## What the engine does

#### Board and moves
* **Bitboards** : a position is 14 integers of 64 bits, one bit per square, one integer per type of piece and per color plus one for all the pieces of each color
* **A move is 16 bits** : starting square, arrival square and a flag _(castle, en passant, pawn moved two squares, promotion)_
* **Legal move generation** : the attacks of the knights, the kings and the pawns are precomputed, the ones of the rooks and the bishops are walked until the first piece met. Each move is then played and tested : it's kept only if the king isn't in check afterwards
* **Log of the game** : every move with the piece it captured and the castling rights after it. It's what allows undoing a move, and the castling rights and the en passant are read from it
* **Positions** set from the starting position and a list of moves, or from a FEN

#### Search
* **Negamax with alpha-beta pruning** : the branches that can't change the result aren't searched
* **Iterative deepening** : depth 1, 2, 3, ... until the time is over. The move played is the one of the last depth completed
* **Quiescence search** : at the end of the depth, the captures are searched until the position is quiet, so an exchange isn't judged in its middle
* **Transposition table** indexed by a Zobrist key _(64 MB by default)_ : a position already searched isn't searched again, and its best move is tried first the next time
* **Move ordering** : the move of the transposition table, then the castle, the winning captures, the promotions, the losing captures. 81 % of the cutoffs are made by the first move tried
* **Extensions** of one half-move for a check and for a pawn reaching the 7th rank
* **Late move reductions** : the quiet moves far in the list are first searched one half-move less, and searched again fully only if they turn out better than expected
* **Draw detection** : a position repeated or 50 moves without progress are seen as draws inside the search, so the engine doesn't repeat the moves in a won position
* **Mate scores with the distance** : the shortest mate is chosen

#### Evaluation
In hundredths of a pawn, seen from the side to play :
* **Material** : pawn 100, knight 300, bishop 320, rook 500, queen 900
* **Piece square tables** : a bonus for each piece depending on its square, with a second table for the endgame for the pawns and the king, blended as the pieces are exchanged
* **Pawn structure** : passed pawns _(up to 120 near the promotion)_, isolated pawns
* **King safety** : pawns in front of the castled king, penalty for a king left in the center, open files near the king, all of it weighted by what the opponent has left to attack
* **Won endgames** : the king is brought close to the opponent king to help the mate

#### Opening book
* 3536 openings of 12 half-moves taken from 7756 master games, loaded in memory at the start
* A move is picked at random among the ones the book knows in the position, on the first 10 half-moves by default : it gives a direction to the opening and makes the games different, without reciting whole games

#### Time
* With the clocks of the game : `time remaining / 30 + 3/4 of the increment`, never more than half of the time remaining, minus a margin of 30 ms for the communication
* The clock is read every 1024 positions, and the search can be stopped at any moment with the `stop` command


## What was done in this version
1. **Separating the engine from the interface.** The AI and the shared helper functions included SDL, so the engine couldn't be compiled without it. It was isolated so it compiles and runs alone, first checked with `Engine_Check`, a program that makes it play against itself in a terminal
2. **Checking the rules.** A perft program was written and compared to the known values on 7 positions. It found the bugs of the move generation, corrected until every count was right
3. **Hunting the memory errors**, with the address and undefined behavior sanitizers of the compiler, on games of the engine against itself
4. **Adding the UCI protocol**, the standard way to talk to a chess engine, with the search in its own thread
5. **Building the benchmark** : tournaments with Fastchess against Stockfish limited in strength, Elo estimation with a confidence interval, comparison of two versions with a SPRT, Markdown reports, resuming of an interrupted campaign
6. **Measuring** : 1673 ± 89 for the corrected engine _(version 1.0)_
7. **Improving the engine one change at a time**, each one validated by a SPRT against the previous version before being kept
8. **Measuring again** : 1983 ± 63 for the version 1.2
9. **Removing the graphical game**, and documenting every folder
10. **Correcting the mop up bonus** after the measures : it isn't measured _(see [Evolution of the versions](#evolution-of-the-versions))_


## Corrections made to the engine
The engine first had to be made reliable : against another program, a crash or an illegal move loses the game, and a wrong rule changes the level measured. A few games against a human in a graphical interface hide most of these bugs, hundreds of games against another program don't.

#### Move generation _(found by the perft)_
| Bug | Consequence |
|---|---|
| The white long castle sent the king to the wrong square | a wrong position after `O-O-O` |
| The black long castle tested its own pieces instead of the white ones on the squares crossed by the king | the castle was tested against the wrong attackers |
| The promotions of the black pawns were tested with the white color, at two places | wrong black promotions |
| The en passant capture of a white pawn to the left wasn't flagged as an en passant and captured the wrong piece | a wrong position after the capture |
| A promotion to a rook, a bishop or a knight played by the opponent wasn't understood | the engine lost track of the position |
| A shift of 64 bits on a 64 bits integer in the pawn moves and the pawn masks | undefined in C, the result depends on the compiler |

#### Search _(found by the sanitizers and by reading the code)_
| Bug | Consequence |
|---|---|
| The array of moves wasn't freed on a beta cutoff | a memory leak at every cutoff, thousands per second |
| The extensions could go over their limit, and the pawn extension looked at the wrong square | lines extended for nothing |
| A mate was scored the same whatever its distance | the engine didn't choose the shortest mate |
| The search answered a null move when there was no legal move at the root | an illegal move sent to the opponent |
| The log of the game was limited to 250 half-moves | a long game wrote outside of the array _(2048 now)_ |
| The sign of « captured piece minus capturing piece » was inverted in the move ordering | the worst captures were searched first |

#### Transposition table
| Bug | Consequence |
|---|---|
| The table of random numbers was too small for the number of bitboards | reads outside of the array, wrong keys |
| The size asked in MB was multiplied by the size of an entry | a table 24 times bigger than asked |
| The best move of a position was erased when a new search didn't find a better one | the move to try first was lost |

#### Evaluation
| Bug | Consequence |
|---|---|
| The pawn shield was evaluated for a king in the center, and the penalty of the king not castled was given to the castled king | the engine avoided castling : in one of its losses it played `Kf8`, it castles now |
| A conversion to an integer put at the wrong place cancelled the endgame tables of the pawns and of the king until the endgame was total | the endgame tables had almost no effect |
| In the won endgames, the bonus meant to push the opponent king to the edge read the square of the friendly king _(corrected after the measures, not measured)_ | nothing pushed the losing king to the edge, where it can be mated, and the friendly king was sent away from it |

#### Opening book
| Bug | Consequence |
|---|---|
| A line with a move that wasn't understood _(`N1c3`, `Nbxd2`, or a pinned knight)_ gave a random move | a crash of the engine. These lines are skipped now _(20 out of 3556)_ |
| The book was read and rewritten on the disk during the game | files written at every move, shared by all the engines running at the same time. It's loaded once in memory now |


## Improvements validated one by one
A change of a chess engine that looks good can make it weaker. So each change played against the previous version, with the same openings and the colors reversed, until a sequential test _(SPRT)_ concluded.

| Change | Games | Elo difference (95 % interval) | Verdict |
|---|---:|---:|---|
| Draw detection in the search _(repetition, 50 moves)_ | 216 | +45 [+14 ; +77] | improvement |
| Move of the transposition table tried first | 274 | +116 [+80 ; +153] | improvement |
| Ageing of the transposition table, best move kept | 364 | +65 [+36 ; +94] | improvement |
| Mate scores with the distance | 400 | +1 [−27 ; +29] | not conclusive, kept as a correction |
| Speed of the search | 209 | +132 [+95 ; +173] | improvement |
| King safety corrected | 400 | +23 [−7 ; +52] | not conclusive alone |
| Middle game / endgame blend corrected | 400 | +15 [−14 ; +44] | not conclusive alone |
| The two evaluation corrections together | 600 | +44 [+19 ; +69] | improvement |

* **Move of the transposition table first** : the number of positions searched for the same depth went from 4.5 million to 1.5 million on the bench
* **Speed** : from 517 000 to about 1 000 000 positions per second, with exactly the same positions searched. It came from a faster way to find the piece on a square, the en passant test made only when it's possible, the clock read every 1024 positions instead of every position, and the link time optimization of the compiler
* **The two evaluation corrections** weren't conclusive one by one, so they were tested together on more games


## How the level is measured
The full method is in [Benchmark](./Benchmark/). In short :

1. **The opponent** is Stockfish with `UCI_LimitStrength`, at 6 levels of `UCI_Elo` : 1320, 1400, 1600, 1800, 2000 and 2200. Several levels are needed, because a score close to 100 % against one opponent says almost nothing
2. **The tournament** is run by Fastchess : the same time control for everybody (8 s + 0.08 s per move), the colors alternated, 4 games at the same time, every game saved in a PGN file
3. **The opening book of the engine** stays active on its first 10 half-moves, which gives different games
4. **The Elo** is the rating that makes the sum of the expected scores equal to the sum of the scores obtained against all the levels, with `E = 1 / (1 + 10^((R_opponent - R) / 400))`
5. **The uncertainty** is always given, as a 95 % confidence interval : about ±90 for 120 games, ±60 for 300, ±25 for 1500
6. **The invalid games** are treated apart : a game lost by the opponent for a technical reason is removed, a technical loss of the engine counts as a loss and is reported
7. **Two versions of the engine** are compared by making them play each other, with a SPRT _(elo0 = 0, elo1 = 10, alpha = beta = 0.05)_ that stops as soon as the result is established
8. **Everything is saved** : the configuration, the fingerprint of the engine, the games and the report of each campaign, to compare with the next ones


## Results

#### Version 1.2 against Stockfish limited in strength
| Metric | Result |
|---|---:|
| Estimated Elo (95 % confidence interval) | 1983 ± 63 |
| Games counted | 293 |
| Wins | 207 (70.6 %) |
| Draws | 6 (2.0 %) |
| Losses | 80 (27.3 %) |
| Technical losses of the engine | 0 |

| Opponent | Games | W | D | L | Score |
|---|---:|---:|---:|---:|---:|
| Stockfish 1320 | 49 | 48 | 0 | 1 | 98.0 % |
| Stockfish 1400 | 49 | 46 | 1 | 2 | 94.9 % |
| Stockfish 1600 | 49 | 42 | 0 | 7 | 85.7 % |
| Stockfish 1800 | 50 | 30 | 1 | 19 | 61.0 % |
| Stockfish 2000 | 48 | 22 | 1 | 25 | 46.9 % |
| Stockfish 2200 | 48 | 19 | 3 | 26 | 42.7 % |

The engine wins almost everything up to 1600, is ahead of 1800, and is close to even with 2000 and 2200 : these three last levels are the ones that place it.

**This number favors the engine** : the games are short and most of them never reach the endgame. Of the 300 games played, 238 were stopped by the resign adjudication _(both engines agree on an advantage of 7 pawns or more during 4 moves)_ and the median game is 72 half-moves long. A clear advantage in the middle game is counted as a win without being played until the end, which is exactly the phase where the engine is the weakest _(see [Tested in real conditions](#tested-in-real-conditions))_. Its real level, in games played until the end, is probably lower than 1983, closer to the 1900 seen by hand.

#### Evolution of the versions
| Version | Content | Estimated Elo | Games | Positions per second | Nodes of the bench |
|---|---|---:|---:|---:|---:|
| 1.0 | the original engine with its bugs corrected | 1673 ± 89 | 118 | 660 152 | 4 496 502 |
| 1.1 | + draw detection in the search | not measured _(+45 against 1.0 by SPRT)_ | | | |
| 1.2 | + the other improvements | 1983 ± 63 | 293 | 1 361 442 | 1 445 569 |
| 1.2 + mop up | + the [mop up bonus](#evaluation-1) corrected | not measured | | | 1 447 455 |

* From 1.0 to 1.2 : +310 Elo, with a combined margin of ±109
* The correction of the mop up bonus came after the measures : the 1983 is the one of the engine without it, which is what the bench of 1 445 569 nodes identifies in the reports. The code of this repository gives 1 447 455 nodes. It should mostly help in the won endgames, which the adjudication rarely lets the games reach
* The engine as it was before the corrections wasn't measured : it crashed on some lines of its opening book and generated illegal moves, which loses the game against another engine

#### Tested in real conditions
Games played by hand in [BanksiaGUI](#playing-against-it) against its Stockfish 15.1 limited in strength, with the opening book of the interface. These are a few games, not a statistical measure like the benchmark :

| Time control | Book of the interface | Stockfish | Result |
|---|---|---|---|
| 1 minute | 10 half-moves | `UCI_Elo` 1800 | the engine wins |
| 2 minutes | 20 half-moves | `UCI_Elo` 1900 | the engine wins |
| 2 minutes | 20 half-moves | `UCI_Elo` 2000 | the engine doesn't win, even when it's ahead in the middle game |

* It's consistent with the benchmark _(about 1980)_ : the engine is around the level of Stockfish limited at 1900 to 2000
* **The weak point is the endgame** : with an advantage in the middle game, the engine often doesn't convert it. Its evaluation of the endgames is simple _(a few king and pawn terms, no knowledge of the typical endgames)_, and the longer the game, the more it counts

#### How to read these numbers
* This Elo depends on the time control, on the machine and on the calibration of Stockfish limited in strength : it's used to follow the engine over time, it isn't an official rating, and it isn't the rating a human would have
* It also depends on the adjudication : most games are stopped before the endgame, where the engine is weaker than in the middle game, so the estimate is higher than its level in games played until the end
* The differences measured between two close versions of the engine don't add up exactly against another opponent : the SPRT gains sum to about +400 from 1.0 to 1.2, the measure against Stockfish gives +310
* The reports of every campaign are in [Benchmark/results](./Benchmark/results/) _(written in French)_


## Limitations

#### Rules
* The AI only looks at the promotions to a queen during its search _(the other promotions are understood when the opponent plays them)_
* The draw by insufficient material isn't taken into account

#### Search
* No null move pruning, no killer moves, no history heuristic, no pruning in the quiescence search _(where 79 % of the positions searched are)_ : these are the next improvements to make if needed
* The array of moves is allocated at every position, and the Zobrist key is recomputed from the whole board
* The time given to a move doesn't depend on the position
* One thread only

#### Known defects, left as they are
Found while documenting the code. Correcting them changes the play of the engine, so each one would have to be validated by a SPRT :
* The transposition table never gives back a score of exactly 0 or −1 : these positions are searched again

#### Seen in the games
* The engine converts its advantages badly in the endgame : it can be ahead in the middle game and fail to win _(see [Tested in real conditions](#tested-in-real-conditions))_. These games were played before the correction of the mop up bonus

#### Measure
* One time control, one machine, one family of opponents
* ±63 Elo with 300 games : a more accurate number needs many more games


## Structure of the code
* **One module is a `.h` and a `.c` with the same name.** The `.h` starts with a summary of the module and the list of its functions, then each function has its description and its parameters. The `.c` only has the code, commented step by step
* **The names say what the thing is**, in full words : `generate_all_possible_moves_that_capture`, `is_search_stop_requested`, `position_halfmove_clocks`
* **The comments explain why**, in particular where a bug was corrected : the reason is written next to the line, so the same mistake isn't made again

```c
/////////////////////////////////////////////////////////////////////////////////////
// Description of what the function is doing
/**
 * @param parameter_1 : description of parameter_1
 * @param parameter_2 : description of parameter_2
 * @return what the function gives back
**/
/////////////////////////////////////////////////////////////////////////////////////
header of the function
```
