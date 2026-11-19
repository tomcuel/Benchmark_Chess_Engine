## Folder containing the functions that make the AI search and play

This folder chooses the move to play. It takes the position from the [Game_Treatment](../Game_Treatment/) folder _(the bitboards, the log, the color to play)_ and answers with a move : one of the opening book when the game is still in it, the result of the search otherwise.

#### Table of contents
* [Path tree](#path-tree)
* [How a move is chosen](#how-a-move-is-chosen)
* [Iterative deepening](#iterative-deepening)
* [The search](#the-search)
* [Quiescence search](#quiescence-search)
* [Draw detection](#draw-detection)
* [Transposition table](#transposition-table)
* [Move ordering](#move-ordering)
* [Evaluation](#evaluation)
* [Opening book](#opening-book)
* [Using the search from another program](#using-the-search-from-another-program)
* [Limitations](#limitations)


## Path tree
```
Alpha_Beta/
├── Data/ # the files used for the opening book : all the downloaded games, and the openings taken from them (Opening_in_Games.txt, the only file read by the engine)
│
├── AI # the function that makes the AI play : a move of the opening book if there is one, the search otherwise
│
├── Evaluation # the evaluation of a position, the heuristic used at the end of the search tree
│
├── Move_Ordering # ordering the moves so the ones most likely to be the best are searched first, which improves the alpha-beta pruning
│
├── Openings # loading the opening book and choosing a move in it
│
├── Pieces_Squares_Tables # the tables that give the preferred squares of each piece (depending on the game phase)
│
├── Pre_Computed_Data # the data computed once at the start : distances between the squares, pawn shield squares of each king square
│
├── Transposition_Table # the Zobrist keys and the table that stores the positions already searched with their value
│
└── Search # the search algorithms : iterative deepening, alpha-beta pruning, quiescence search, extensions, reductions, draw detection
```


## How a move is chosen
`make_AI_play` _(AI.c)_ is the entry point :

1. `get_move_from_book` looks for a move in the opening book. If there is one, it's played at once
2. Otherwise `run_iterative_deepening` searches the position until the time is over
3. The move is made on the board with `make_move_AI`, and the bitboards are added to the log of the bitboards
4. When no move is found _(checkmate or stalemate)_, the selected move stays null and nothing is played

The UCI program calls `get_move_from_book` and `run_iterative_deepening` directly, because it answers with the move and lets the other program play it.


## Iterative deepening
`run_iterative_deepening` searches at depth 1, then 2, then 3, ... :

* **Only a completed depth counts.** When the time ends in the middle of a depth, its result is thrown away and the move of the previous depth is played. So the move played always comes from a full search
* **Each depth prepares the next one** : the best moves it found are in the transposition table and are tried first at the next depth, which makes the whole thing faster than searching the last depth directly
* **The search stops early** when a mate is found _(a score above `IMMEDIATE_MATE_SCORE - 1000`)_ or when `forced_max_depth` is reached
* **Backup** : if not even the depth 1 is completed in time, a depth 1 search is run without time limit, so there is always a move to play
* **Time** : `think_time_limit` is `forced_think_time_in_ms` when it's set _(the UCI program always sets it)_, otherwise `choose_think_time`. The clock is only read once every 1024 nodes, because reading it at every node costs time. `is_search_stop_requested` stops the search from another thread


## The search
`Search(ply_remaining, ply_from_root, alpha, beta, nb_extensions)` is a **negamax** : the score is always seen from the side to play, and the score of a move is the opposite of the score of the position it gives to the opponent. The **alpha-beta pruning** is fail-hard : the function never returns a value outside of `[alpha, beta]`.

At each node, in this order :

1. **Stop** if the time is over
2. **Mate distance pruning** : a mate already found closer to the root can't be beaten here, the window is tightened and the node is left if it becomes empty
3. **Draw** : a position repeated or 50 moves without capture or pawn move gives 0 _(see below)_
4. **Transposition table** : if the position was already searched at least as deep and its value can be used in the current window, it's returned at once
5. **Depth reached** : the quiescence search takes over
6. **Generation of the legal moves.** No move means checkmate `-(IMMEDIATE_MATE_SCORE - ply_from_root)` if the king is in check, stalemate `0` otherwise
7. **Ordering of the moves**, with the move of the transposition table first
8. **Loop on the moves** : the move is made, searched, undone
9. **Storing** the result in the transposition table

#### In the loop on the moves
* **Extension** : the move is searched one half-move deeper when it gives check, or when a pawn arrives on the 7th rank _(the 2nd for black)_. A line can't be extended more than 16 times _(`MAX_NUMBER_OF_EXTENSIONS`)_, so the search always ends
* **Late move reduction** : a move that isn't extended, isn't a capture, comes after the first 3 moves of the list and has at least 3 half-moves left is first searched one half-move less, with a window of one point `[-alpha-1, -alpha]`. It only has to say « better than alpha or not ». If it's better, it's searched again at the full depth with the full window
* **Beta cutoff** : when a move scores at least beta, the opponent won't allow this position, the other moves aren't searched. The move is stored in the table as a `BETA_CUT`
* **New best move** : when a move scores above alpha, alpha is raised and the move becomes the best of the position. At the root, it becomes the move of the depth

#### Mate scores
A mate is worth `IMMEDIATE_MATE_SCORE - ply_from_root` _(100 000 minus the number of half-moves from the root)_. A mate in 1 is worth more than a mate in 3, so the engine goes for the fastest mate and delays the one it suffers as much as it can.

#### Counters
`search_statistics` counts the nodes, the quiescence nodes, the hits in the transposition table, the beta cutoffs and the ones made by the first move tried. They are reset at each search and shown by the `bench` command of the UCI program.


## Quiescence search
Stopping the search at a fixed depth gives wrong scores in the middle of an exchange : the queen that has just captured a pawn looks like a pawn won, when it's taken back on the next move. `Quiescience_Search` continues until the position is quiet :

1. The position is evaluated : this « stand pat » score is what the side to play gets if it captures nothing. If it's already at least beta, the node is cut
2. Only the captures are generated and ordered
3. Each capture is made, searched with the same function, undone, with the same alpha-beta pruning
4. There is no depth limit : it ends because the number of pieces to capture is limited

It doesn't use the transposition table, and it doesn't look at the checks. It's where the engine spends most of its time : 79 % of the nodes of the bench are quiescence nodes.


## Draw detection
The transposition table says what a position is worth, but not that it has already been played in the game. Without this, the engine repeats the moves in a won position and gives the draw away.

* **Before the search**, `prepare_position_history` undoes the last moves of the game, back to the last capture or pawn move _(no position from before it can come back)_, and plays them again while keeping the Zobrist key of each position in `position_keys`. The state of the game is compared to a copy at the end : if anything differs, it's put back and the draws are ignored for this search
* **During the search**, each node stores its key, and `is_position_repeated` goes back through the keys, one position out of two _(same side to play)_, and stops at the first capture
* **One repetition is enough** to score 0 : if a player can repeat once, he can repeat again
* **50 moves rule** : `position_halfmove_clocks` counts the half-moves without capture or pawn move, 100 gives a draw. A position loaded from a FEN starts with the count of the FEN
* This test is made **before** the transposition table, because the value stored there doesn't know the positions already played
* `is_draw_detection_enabled` turns it off _(it was used to measure what it brings : +45 Elo)_


## Transposition table
The same position is reached by different orders of moves. The table keeps what was found the first time.

#### Zobrist key
`get_zobrist_key` gives a 64 bits number for the position, by combining with XOR one random number for :
* each piece on each square _(64 × 14 numbers)_
* the side to play
* the castling rights _(one number per value of the 6 bits of rights)_
* the file of the en passant, when the last move is a pawn moved two squares

The key is recomputed from the whole board at each node, it isn't updated move by move.

#### An entry
| Field | Content |
|---|---|
| `zobrist_key` | the full key, to check that it's the right position |
| `depth` | the number of half-moves searched under this position |
| `value` | the score |
| `cut_flag` | `EXACT` : the real score, `BETA_CUT` : the score is at least this value, `ALPHA_CUT` : the score is at most this value |
| `move` | the best move found |
| `age` | the number of the search that wrote the entry |

* The index is `key % size`, with one entry per index : two positions with the same index replace each other
* An entry is 24 bytes, the option `Hash` gives the size in MB _(64 by default, about 2.8 million entries)_

#### Reading
`lookup_for_evaluation_in_TT` gives the value only when the key is the same, the stored depth is at least the one asked, and the flag allows it : an `EXACT` value always, a `BETA_CUT` value when it's at least beta, an `ALPHA_CUT` value when it's at most alpha.

Even when the value can't be used, **the move of the entry is used** : it's given to the move ordering to be tried first.

#### Writing
* An entry is replaced when the new search is at least as deep, or when it was written by a previous search _(the age changes at each search with `start_new_search_in_TT`)_. Without the age, a deep entry of a position left 20 moves ago would stay forever
* When the new search has no best move for a position already there _(no move was better than alpha)_, the previous best move is kept
* **Mate scores** are counted from the root when they are in the search, and from the position when they are in the table : `correct_mate_score_for_storage` and `correct_retrieved_mate_score` convert them, so a mate found at one depth stays right when the position is met at another depth


## Move ordering
The alpha-beta pruning cuts much more when the best move is searched first. `order_moves` gives a score to each move and sorts them with a quick sort :

| Move | Score |
|---|---:|
| the move of the transposition table | + 1 000 000 |
| castle | + 10 000 |
| capture that can't be taken back, or that doesn't lose material | + 8 000 + _(captured piece − capturing piece)_ |
| promotion without capture | + 6 000 |
| capture that can be taken back and loses material | + 2 000 + _(captured piece − capturing piece)_ |
| arrival square attacked by an opponent piece | − 25 |
| arrival square attacked by an opponent pawn | − 50 |

* The values of the pieces are 100, 300, 320, 500 and 900, so a pawn that captures a queen gets `8 000 + 800` and a queen that captures a defended pawn gets `2 000 − 800`
* In the quiescence search, only the scores of the captures and the pawn penalty are computed
* Measured by `bench 6` : 81 % of the beta cutoffs are made by the first move tried


## Evaluation
`evaluate` gives the score of a position in hundredths of a pawn, seen from the side to play. It's computed as white minus black, then the sign is changed if black is playing.

#### Game phase
Each side has an **endgame transition** between 0 _(middle game)_ and 1 _(endgame)_, computed from its own pieces without the pawns : queen 45, rook 20, bishop 10, knight 10. With the pieces of the start _(125)_ the transition is 0, and it grows as the pieces are exchanged.

#### The terms
| Term | What it measures |
|---|---|
| Material | pawn 100, knight 300, bishop 320, rook 500, queen 900 |
| Piece square tables | the bonus of each piece for the square it's on |
| Pawns | passed pawns and isolated pawns |
| King safety | pawn shield, king left in the center, open files near the king |
| Mop up | the king helps to mate in a won endgame |

* **Piece square tables** : one table per piece and per color. The pawns and the kings have a second table for the endgame, and their score is `transition × endgame table + (1 − transition) × middle game table` : the king hides in a corner in the middle game and comes to the center in the endgame, the pawns are pushed in the endgame
* **Passed pawn** : no opponent pawn in front of it or on the two files next to it. The bonus grows as it gets closer to the promotion : 15, 15, 30, 50, 80, 120
* **Isolated pawns** : no friendly pawn on the two files next to it. The penalty depends on their number : 10, 25, 50, then 75
* **Pawn shield** : when the king is on the files a, b, c or f, g, h, each pawn missing in front of it gives a penalty, smaller when the pawn is only one square further. The sum is squared, so two missing pawns cost much more than twice one
* **King in the center** : when the king is on the files d or e, a penalty up to 50, bigger when the opponent pieces are developed
* **Open files** : when the opponent has two rooks or a rook and a queen, a file near the king without opponent pawn costs 15 to 25, and 10 to 15 more without friendly pawn on it
* The whole king safety is multiplied by `1 − transition of the opponent`, and by 0.6 more when the opponent has no queen : a king doesn't need a shield when there is nothing left to attack it
* **Mop up** : when a side is ahead by more than two pawns and the opponent has few pieces left, a bonus for bringing its king close to the opponent king and for the opponent king being far from the center, weighted by the transition. The second part read the square of the friendly king until it was corrected after the measures of the version 1.2

`Pre_Computed_Data` holds what these terms read : the distance between two squares, the distance of a square to the center, and for each king square the squares where the pawns of its shield should be.


## Opening book
* `Data/Download_Games.txt` holds 7756 master games, and `Data/Opening_in_Games.txt` the 3556 different openings of 12 half-moves found in them, in algebraic notation _(`e4 e5 Nf3 Nc6 ...`)_
* `load_opening_book` reads the file once at the start and converts each line to moves of the engine by playing it on the board. Nothing is written on the disk
* A line with a move that isn't understood or isn't legal is skipped : 20 lines, 3536 are kept _(ex : `N1c3`, `Nbxd2`, or a knight chosen while it's pinned)_
* `get_move_from_book` takes the lines that start with the moves already played, and picks at random one of the moves that follow. So the engine doesn't always play the same opening
* The book is only used while the moves of the log are the start of one of its lines, and on the first 10 half-moves by default
* `set_opening_book_usage(enabled, max_plies)` and `set_opening_book_path` change it _(options `OwnBook`, `BookPlies` and `BookFile` of the UCI program)_, and `set_fixed_seed` replays the same choices _(option `Seed`)_
* Getting the unique openings from the games (ex : 6 complete moves = 12 half-moves) : `sed -E 's/^(([^ ]+ ){11}[^ ]+).*/\1/' Download_Games.txt > All_Openings.txt && sort All_Openings.txt | sed '$!N; /^\(.*\)\n\1$/!P; D' > Opening_in_Games.txt`


## Using the search from another program
The search is driven by global variables declared in `Search.h` :

| Variable | Role |
|---|---|
| `forced_think_time_in_ms` | time of the search in ms, 0 to let `choose_think_time` decide |
| `forced_max_depth` | maximum depth, 0 for no limit |
| `is_search_stop_requested` | set to true from another thread to stop the search |
| `search_iteration_callback` | function called each time a depth is completed, with the depth, the score and the best move |
| `search_statistics` | the counters of the last search |
| `is_draw_detection_enabled` | to turn the draw detection off |

The UCI program uses all of them : the callback writes the `info depth ... score ... pv ...` lines.


## Limitations
* The AI only promotes a pawn to a queen, in the search and in the opening book
* The draw by insufficient material isn't taken into account
* No null move pruning, no killer moves, no history heuristic : the quiet moves are almost not ordered
* No pruning in the quiescence search, where 79 % of the nodes are
* The transposition table never gives back a value of exactly 0 _(`lookup_for_evaluation_in_TT` ignores them)_, so the drawn positions are searched again
* The array of moves and the array of scores are allocated at every node, and the Zobrist key is recomputed from the whole board at every node
* `choose_think_time` uses the clocks of the game settings and gives 5 seconds by default : the UCI program doesn't use it
* The search still prints its debug texts on the standard output _(the UCI program sends them to nowhere unless `debug on`)_
