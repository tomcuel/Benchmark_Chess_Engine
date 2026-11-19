## Folder containing the constants and the functions unrelated to chess

This folder is the base of the engine : every other file includes it, and it includes nothing of the engine. It holds the types and the constants shared by all the folders, and a few small functions _(clock, random numbers, bounds, squares)_ that only need the standard library.

#### Table of contents
* [Path tree](#path-tree)
* [Constants : the types](#constants--the-types)
* [Constants : the values](#constants--the-values)
* [Utility : the clock](#utility--the-clock)
* [Utility : the random numbers](#utility--the-random-numbers)
* [Utility : the squares and the small functions](#utility--the-squares-and-the-small-functions)
* [Limitations](#limitations)


## Path tree
```
Useful/
├── Constants # the types (bit_board, move, castling_rights_flags), the enums and the constants shared by the whole engine
│
└── Utility # the clock, the random numbers, min/max/clamp, and the conversions between a square, its rank and its file
```

* `Constants.h` is a header only, it also includes the standard headers used everywhere _(`stdio.h`, `stdlib.h`, `stdbool.h`, `string.h`, `math.h`, `limits.h`, `time.h`, `unistd.h`)_
* `Utility.c` is the only source of the folder, it's compiled with the engine by the makefile of the [UCI](../UCI/) folder


## Constants : the types
Three types carry almost everything in the engine. They are small integers on purpose : a position and a move are copied, compared and stored millions of times per second.

| Type | Size | Content |
|---|---|---|
| `bit_board` | 64 bits | one bit per square of the board |
| `move` | 16 bits | bits 0 to 5 : starting square, bits 6 to 11 : arrival square, bits 12 to 15 : flag |
| `castling_rights_flags` | 8 bits | 6 bits used, one per king and per rook, set when the piece has moved |

How they are used is described in [Game_Treatment](../Game_Treatment/).


## Constants : the values

#### The board and the pieces
| Constant | Value | Role |
|---|---:|---|
| `BOARD_SIZE` | 8 | ranks and files |
| `NUMBER_SQUARES` | 64 | squares |
| `NUMBER_OF_BIBOARD` | 14 | bitboards of a position |
| `WHITE`, `BLACK`, `NO_COLOR` | 0, 1, 2 | the colors. `1 - color` gives the other one |

The enum of the pieces gives the index of each bitboard : `PAWN_WHITE` (0) to `KING_WHITE` (5), `WHITE_PIECES` (6), `PAWN_BLACK` (7) to `KING_BLACK` (12), `BLACK_PIECES` (13), and `NOTHING` (14) for an empty square. A black piece is the white one `+ 7`.

#### The masks and the flags
* `start_square_mask`, `target_square_mask`, `flag_mask` : the three parts of a move
* `File_A_mask`, `Full_Bitboard_mask`, `Empty_Bitboard_mask` : bitboards of the first file, of everything, of nothing. `File_A_mask << file` gives any file
* The 8 flags of a move, from `No_Flag` (0) to `Promote_To_Bishop_Flag` (7)
* The 6 flags of the castling rights, from `White_King_Moved_Flag` to `Black_Rook_H_Moved_Flag`. `All_Castling_Rights` is 0 : no piece has moved

#### The state of the game
`NO_CHECK`, `CHECK`, `CHECK_MATE`, `STALEMATE`, `NO_DRAW`, `DRAW_BY_3_RULES`, `DRAW_BY_50_RULES`, `DRAW_BY_INSUFFICIENT_MATERIAL` : what `is_check`, `get_check_state` and `is_draw_forced_by_rules` return.

#### The memory
| Constant | Value | Role |
|---|---:|---|
| `max_size_log_array` | 2048 | half-moves that the log can hold : the game plus the line being searched |
| `max_size_valid_moves_array` | 100 | size of the array of moves of a position, almost never reached |
| `max_size_valid_moves_array_extended` | 218 | the array is extended to this size when it's full : the maximum number of legal moves of a chess position |

The log was limited to 250 half-moves at first : a long game against another engine went out of the array.

#### The search
| Constant | Value | Role |
|---|---:|---|
| `PAWN_VALUE`, `KNIGHT_VALUE`, `BISHOP_VALUE`, `ROOK_VALUE`, `QUEEN_VALUE` | 100, 300, 320, 500, 900 | value of the pieces, in hundredths of a pawn |
| `IMMEDIATE_MATE_SCORE` | 100 000 | score of a mate at the root, minus one per half-move of distance |
| `POSITIVE_INFINITY`, `NEGATIVE_INFINITY` | ± 9 999 999 | the window of the search at the root |
| `EXACT`, `ALPHA_CUT`, `BETA_CUT` | 0, 1, 2 | type of the value stored in the transposition table |
| `FAILED_LOOKUP` | −1 | answer of the transposition table when it has nothing usable |
| `MAX_NUMBER_OF_EXTENSIONS` | 16 | extensions allowed on one line of the search |
| `MAX_DEPTH_ITERATIVE_DEEPENING` | 100 | depth at which the iterative deepening stops anyway |
| `NUMBER_OF_OPENING_MOVES` | 12 | half-moves of a line of the opening book |

The mate score is far above any evaluation _(a side with nine queens is under 10 000)_, and the infinities are far above the mate score : the three ranges can't be mixed up. A score beyond `IMMEDIATE_MATE_SCORE - 1000` is read as a mate everywhere.


## Utility : the clock
`get_time_ms()` gives a time in milliseconds, used to time the search and the bench.

* It reads `CLOCK_MONOTONIC` : a clock that only goes forward, that the system doesn't adjust. With the clock of the date, a time correction during a game could make a search last a negative time or never end
* The value is counted from the first call, not from 1970 : it starts near 0, so it fits in an `int` _(which overflows after 24 days)_
* Only differences between two calls mean something
* The search doesn't call it at every node, it costs too much : once every 1024 nodes


## Utility : the random numbers
The only random choice of the engine is the move taken in the opening book. The search itself is deterministic.

* `custom_rand()` is a linear congruential generator : `seed = (1664525 × seed + 1013904223) mod 2^32`. It's simple and its sequence is the same on every machine, which the C `rand()` doesn't promise
* `random_int(min, max)` gives a number between the two bounds included
* `set_seed()` takes a seed from the time and the number of the process. It's called before each choice in the book, so two games don't follow the same openings, even two engines launched during the same second
* `set_fixed_seed(value)` gives the seed, and the following `set_seed()` are then ignored : the same sequence of choices is played again. It's the option `Seed` of the UCI program, used to replay a game exactly

The Zobrist keys of the transposition table don't use this generator, they use `rand()` without a seed : they are the same at every launch, which makes the bench give the same number of nodes every time.


## Utility : the squares and the small functions
| Function | Result | Computed as |
|---|---|---|
| `get_rank(square)` | the rank, 0 to 7 | `square >> 3` |
| `get_file(square)` | the file, 0 to 7 | `square & 7` |
| `get_index(rank, file)` | the square, 0 to 63 | `(rank << 3) \| file` |
| `divide_by_two(number)` | half of the number | `number >> 1` |
| `min(a, b)`, `max(a, b)` | the smaller, the bigger | |
| `int_clamp(value, min, max)`, `float_clamp(...)` | the value put back between the two bounds | |

* The squares go from a1 = 0 to h8 = 63, so the rank and the file are the two halves of the 6 bits of a square. The shifts and the masks say it directly
* These functions are called from every folder. With the link time optimization _(`-flto`)_ they are inlined where they are called, instead of costing a function call each time
* `int_clamp` is used to keep the values of the UCI options and of the FEN in their range, `float_clamp` by the evaluation


## Limitations
* `FAILED_LOOKUP` is −1, which is also a possible score : a position worth exactly −1 hundredth of a pawn found in the transposition table is treated as not found, and searched again
* `random_int` uses a modulo, so the numbers aren't perfectly uniform. It doesn't matter for a choice among a few moves
* `min` and `max` only exist for the integers
