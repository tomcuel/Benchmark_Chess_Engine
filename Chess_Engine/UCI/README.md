## Folder containing the programs of the chess engine

The folders [Alpha_Beta](../Alpha_Beta/) and [Game_Treatment](../Game_Treatment/) are a library : they don't have a `main`. The three programs are here. `UCI.x` is the engine itself, the one that plays games. `Perft.x` and `Engine_Check.x` are there to check it.

#### Table of contents
* [Path tree](#path-tree)
* [Compilation](#compilation)
* [UCI : the engine](#uci--the-engine)
* [Perft : checking the move generation](#perft--checking-the-move-generation)
* [Engine_Check : a game against itself](#engine_check--a-game-against-itself)
* [Limitations](#limitations)


## Path tree
```
UCI/
├── Engine_Check # makes the AI play against itself in the terminal, to check quickly that the engine works
│
├── Perft # checks the move generation, by counting the positions reached from reference positions and comparing them to the known values
│
├── UCI # the engine with the UCI protocol (Universal Chess Interface), to make it play against other engines with a tournament manager
│
├── makefile # compiles the three programs
│
└── build/ # the objects of the engine, created by the makefile
```


## Compilation
* `make -C Chess_Engine/UCI` _(or `make -C Chess_Engine`)_ compiles the three programs
* `make -C Chess_Engine/UCI realclean` removes the objects and the programs
* The sources of `Alpha_Beta`, `Game_Treatment` and `Useful` are compiled into `build/`, then linked with each program
* The flags are `-O2 -flto` : `-flto` lets the compiler optimize across the files, so the small functions called everywhere _(`get_type_of_piece`, `get_flag`, ...)_ are inlined. It was a part of the speed gain measured at +132 Elo
* Only the standard library and the threads are needed _(`-lm -lpthread`)_
* The compiler writes the headers each file depends on _(`-MMD`)_, so changing a `.h` recompiles everything that includes it


## UCI : the engine
UCI _(Universal Chess Interface)_ is the protocol that the chess interfaces and the tournament managers use to talk to an engine. The engine is a program without any window : it reads commands as text on its standard input and answers on its standard output. The other program holds the board, the clocks and the rules, the engine is only asked « here is the position, what do you play ? ».

`./Chess_Engine/UCI/UCI.x` can be launched from any directory, and typed into by hand.

#### An exchange
```
> uci
id name Cleaned Chess Engine 1.2
id author tomcuel
option name Hash type spin default 64 min 1 max 1024
option name OwnBook type check default true
option name BookPlies type spin default 10 min 0 max 12
option name BookFile type string default <empty>
option name Seed type spin default 0 min 0 max 2147483647
option name Move Overhead type spin default 30 min 0 max 5000
uciok
> isready
readyok
> setoption name OwnBook value false
> position startpos moves e2e4 e7e5
> go depth 5
info depth 1 score cp 51 nodes 59 nps 59000 hashfull 0 time 0 pv b1c3
info depth 2 score cp 0 nodes 296 nps 296000 hashfull 0 time 0 pv b1c3
info depth 3 score cp 70 nodes 1067 nps 1067000 hashfull 0 time 1 pv b1c3
info depth 4 score cp 5 nodes 8248 nps 1031000 hashfull 1 time 8 pv f1c4
info depth 5 score cp 27 nodes 36755 nps 1113787 hashfull 2 time 33 pv b1c3
info string stats depth 5 nodes 36755 qnodes 19275 tthits 474 cutoffs 3958 firstmovecutoffs 68%
bestmove b1c3
```
_(the lines starting with `>` are the commands typed, the `>` isn't typed)_

#### Commands
| Command | What the engine does |
|---|---|
| `uci` | gives its name, its author and its options, then `uciok` |
| `isready` | answers `readyok`, even during a search |
| `setoption name <name> value <value>` | changes an option |
| `ucinewgame` | empties the transposition table and puts the starting position |
| `position startpos [moves ...]` | puts the starting position, then plays the moves given |
| `position fen <fen> [moves ...]` | puts the position of the FEN, then plays the moves given |
| `go movetime <ms>` | searches during this time |
| `go depth <n>` | searches until this depth |
| `go wtime <ms> btime <ms> [winc <ms>] [binc <ms>] [movestogo <n>]` | searches with the clocks of the game |
| `go infinite` | searches until `stop` |
| `stop` | stops the search, the engine gives the best move found |
| `quit` | closes the engine |

Three commands aren't in the protocol, they are there for the tests :

| Command | What the engine does |
|---|---|
| `d` | shows the board and the side to play |
| `bench [depth]` | searches 10 fixed positions and shows the counters of the search |
| `debug on` / `debug off` | shows the debug texts of the search on the error output |

The unknown commands are ignored, as the protocol asks.

#### Options
| Option | Default | Role |
|---|---:|---|
| `Hash` | 64 | size of the transposition table in MB, from 1 to 1024. The table is emptied when it changes |
| `OwnBook` | true | use of the opening book |
| `BookPlies` | 10 | number of half-moves taken from the book, from 0 to 12 |
| `BookFile` | | another file for the opening book, it's loaded at once and the number of lines is answered |
| `Seed` | 0 | seed of the random choices in the book, to play the same games again |
| `Move Overhead` | 30 | time in ms kept at each move for the communication with the other program |

#### How the moves are written
* A move is the starting square followed by the arrival square : `e2e4`
* A castle is the move of the king : `e1g1`, `e1c1`, `e8g8`, `e8c8`
* A promotion has the letter of the piece at the end : `e7e8q`, `e7e8n`
* `0000` means that there is no move to play _(checkmate or stalemate)_
* To read a move, the engine generates the legal moves of the position and looks for the one with the same name. So an illegal move can't be played : it's refused with an `info string error` line and the moves that follow are ignored. The promotions to a rook, a bishop and a knight are generated for this, so the engine understands them when the opponent plays them

#### What the engine answers during a search
One `info` line each time a depth is completed, then `bestmove` :

| Field | Content |
|---|---|
| `depth` | the depth completed |
| `score cp <n>` | the score in hundredths of a pawn, seen from the side to play |
| `score mate <n>` | a mate in n moves _(negative when the engine is mated)_, instead of `cp` |
| `nodes` | the number of positions searched since the start of the search |
| `nps` | nodes per second |
| `hashfull` | filling of the transposition table in per thousand, estimated on its first 1000 entries |
| `time` | time since the start of the search in ms |
| `pv` | the best move _(only the first move of the line, not the whole line)_ |

* `info string stats ...` gives the counters at the end of the search : quiescence nodes, hits in the transposition table, beta cutoffs, share of the cutoffs made by the first move
* A move of the opening book is answered without search, with `info depth 0 score cp 0 ... pv <move>` and `info string book move <move>`. The line with a score is there because the tournament managers expect one before each `bestmove`

#### Time given to a move
| Command | Time of the search |
|---|---|
| `go movetime <ms>` | the time asked, minus the Move Overhead |
| `go wtime ... btime ...` | `time remaining / 30 + 3/4 of the increment`, minus the Move Overhead |
| ... with `movestogo <n>` | `time remaining / (n + 1) + 3/4 of the increment`, minus the Move Overhead |
| `go depth <n>` | no limit of time, the search stops when the depth is completed |
| `go infinite` | no limit, only `stop` ends the search |

* With the clocks, the time is never more than half of the time remaining, and never less than 5 ms
* The time is the same for every move : the engine doesn't think longer in a difficult position
* The move played is the one of the last depth completed, see [Alpha_Beta](../Alpha_Beta/#iterative-deepening)

#### How the program is built
* **The search runs in its own thread.** The main thread keeps reading the commands, so `stop` and `isready` are answered during a search. `stop` sets `is_search_stop_requested` and waits for the end of the thread. Every command that changes the position or an option stops the search first
* **The standard output is kept for the protocol.** The search still has its `printf` from the time it was run in a terminal : at the start, the program redirects them to nowhere and writes its answers through a copy of the output. `debug on` sends them to the error output
* **The opening book is found from the place of the program** _(`<...>/UCI/UCI.x` gives `<...>/Alpha_Beta/Data/Opening_in_Games.txt`)_, so the engine works from any directory
* **Each `position` command rebuilds the whole game** : the starting position, then all the moves. The log of the game is then complete, which the search needs to see the repetitions
* **When the input is closed during a search**, the search is finished and its move is given before leaving, so `printf '...go depth 5\n' | ./UCI.x` works

#### The bench
`./Chess_Engine/UCI/UCI.x bench 6` searches 10 fixed positions _(openings, middle games, endgames)_ at depth 6, each one with an empty transposition table and without the opening book, then closes :
```
bench depth 6 positions 10 nodes 1447455 qnodes 1145311 tthits 12032 cutoffs 54873 firstmovecutoffs 81% time 1309 nps 1105771
```
* **The number of nodes only depends on the code**, not on the machine or on the time. If it changes between two versions, the search or the evaluation has changed. If it's the same, the two versions play exactly the same moves and only the speed can differ
* `nps` gives the speed of this machine
* The benchmark saves this line in each of its reports

#### Using the engine
* To play against it in a chess interface that speaks UCI _(BanksiaGUI, Cute Chess, En Croissant, Arena, ...)_ : adding `Chess_Engine/UCI/UCI.x` as an engine. The steps with BanksiaGUI are in the [main README](../../README.md#playing-against-it)
* What an interface sends is what is described above : `uci`, `isready`, `ucinewgame`, then for each move `position startpos moves ...` and `go wtime ... btime ... winc ... binc ...`. The engine was checked with this sequence, launched from another directory, with a game going through the opening book, a search with the clocks and a promotion to a knight played by the opponent
* Against other engines : with Fastchess or Cute Chess, it's what the [Benchmark](../../Benchmark/) folder does


## Perft : checking the move generation
A perft counts all the positions that can be reached in a given number of half-moves from a position. The right numbers are known for some reference positions : if the engine finds the same, its move generation and its way to make and undo a move follow the rules. One wrong move anywhere changes the count.

#### All the reference positions
`./Chess_Engine/UCI/Perft.x`

| Position | What it tests | Depth | Positions |
|---|---|---:|---:|
| Start position | the basic moves | 5 | 4 865 609 |
| Kiwipete | castles, pins, en passant | 4 | 4 085 603 |
| Endgame | en passant that uncovers the king, pins | 6 | 11 030 083 |
| Promotions and checks | promotions, checks | 4 | 422 333 |
| The same one mirrored | the same thing for the other color | 4 | 422 333 |
| Promotion with capture | promotion by capturing, castle | 4 | 2 103 487 |
| Middle game | an ordinary position | 4 | 3 894 594 |

Each depth is compared, and the program ends with :
```
28026871 positions counted in 1165 ms
the move generation is correct on all the reference positions
```
Its exit code is 0 when everything is right, so it can be used in a script _(`run_sprt.sh` of the benchmark runs it before each comparison)_.

#### Finding a wrong move
`./Chess_Engine/UCI/Perft.x 3 "<fen>"` gives the count under each first move :
```
e5c6: 1
e5g6: 1
...
Nodes searched: 48
```
The output has the same format as the `go perft` command of Stockfish. When a total is wrong, comparing the two lists gives the first move where the counts differ. Playing this move and starting again one depth lower leads to the position where a move is missing or added.

#### Particularities
* The promotions to a rook, a bishop and a knight are generated here _(`are_under_promotions_generated`)_, since the known values count them
* The mirrored position is there because the code of the white and of the black moves is written twice : a bug on one color only, like the black promotions tested with the white color, shows as a difference between the two


## Engine_Check : a game against itself
`./Chess_Engine/UCI/Engine_Check.x [think time in ms] [number of half-moves] [seed]`, from the root of the project to find the opening book. By default 500 ms, 14 half-moves.

* It initializes the engine, loads the opening book and makes the AI play both sides with `make_AI_play`
* It writes each move with where it comes from _(book or search)_, its score and the depth reached, with all the debug texts of the search
* It's the quickest way to see that the engine starts and plays after a change, without any other program. It was the first program written to run the engine without its graphical game
* Example : `./Chess_Engine/UCI/Engine_Check.x 1000 16 42`


## Limitations
* The AI only looks at the promotions to a queen during its search
* A position loaded from a FEN doesn't use its last field _(number of the move)_
* `pv` only gives the best move, not the line that follows
* One thread only, no pondering _(thinking during the time of the opponent)_, and the options `Threads` and `Ponder` don't exist
* The time given to a move doesn't depend on the position
* `go` with `nodes`, `mate` or `searchmoves` isn't supported : the search then runs without limit until `stop`
