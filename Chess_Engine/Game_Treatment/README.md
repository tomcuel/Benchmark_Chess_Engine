## Folder containing the functions linked to the game state and to the moves

Everything the search needs to know about chess is here : how a position is stored, which moves are legal, how a move is made and undone. The search of the [Alpha_Beta](../Alpha_Beta/) folder only calls four functions of this folder at every node : `generate_all_possible_moves`, `make_move_AI`, `undo_last_move_AI` and `is_check`.

#### Table of contents
* [Path tree](#path-tree)
* [How a position is stored](#how-a-position-is-stored)
* [How a move is stored](#how-a-move-is-stored)
* [The log of the game](#the-log-of-the-game)
* [Attacks of the pieces](#attacks-of-the-pieces)
* [Move generation](#move-generation)
* [Making and undoing a move](#making-and-undoing-a-move)
* [End of the game](#end-of-the-game)
* [Positions loaded from a FEN](#positions-loaded-from-a-fen)
* [Checking the move generation](#checking-the-move-generation)
* [Limitations](#limitations)


## Path tree
```
Game_Treatment/
├── Board # the bitboards, the captured pieces and the score, how make_move and undo_last_move change the structures, the draw rules
│
├── Fen # setting up the board from the starting position or from a FEN (pieces, color to play, castling rights, en passant square)
│
├── Log # the log of the game : the move, the captured piece and the castling rights after each move, and the log of the bitboards
│
├── Move_Generator # generating all the legal moves, the state of the game (check, checkmate, draw), and the functions of the AI to make and undo a move
│
├── Moves_Treatment # the encoding of a move, the precomputed attacks, the legality of a move, the castle, the en passant and the promotion
│
└── Settings # the game settings : the player and the color to play, the clocks
```

The state of the game is kept in global variables, shared by all the files : `bitboards`, `chess_log`, `bitboards_log`, `captured_pieces_and_score` and `game_settings`. A function never receives « the position » as a parameter, it reads it there.


## How a position is stored

#### The squares
A square is a number from 0 to 63 : a1 = 0, h1 = 7, a8 = 56, h8 = 63. So `rank = square >> 3` and `file = square & 7`, going up one rank is `+8` and going right one file is `+1`.

#### The bitboards
A bitboard is a 64 bits integer _(`bit_board`, a `uint64_t`)_ with one bit per square. The position is an array of 14 bitboards :

| Index | Bitboard | Index | Bitboard |
|---:|---|---:|---|
| 0 | PAWN_WHITE | 7 | PAWN_BLACK |
| 1 | KNIGHT_WHITE | 8 | KNIGHT_BLACK |
| 2 | BISHOP_WHITE | 9 | BISHOP_BLACK |
| 3 | ROOK_WHITE | 10 | ROOK_BLACK |
| 4 | QUEEN_WHITE | 11 | QUEEN_BLACK |
| 5 | KING_WHITE | 12 | KING_BLACK |
| 6 | WHITE_PIECES _(all of them)_ | 13 | BLACK_PIECES _(all of them)_ |

* The index of a black piece is the one of the white piece `+ 7`, and `NOTHING` (14) means an empty square
* The two bitboards with all the pieces of a color are kept up to date at every move. They answer at once to « is this square empty ? » and « is there one of my pieces here ? »
* `get_type_of_piece` is the most called function of the search : it first looks at these two bitboards, so an empty square is known in one test and the other squares only need the 6 bitboards of the right color

#### What isn't in the bitboards
* **The color to play** is in `game_settings`, with two fields that must stay consistent : `color_of_player_that_is_playing` _(WHITE, BLACK)_ and `player_that_is_playing`. `change_player_that_is_playing` changes both
* **The castling rights and the en passant square** are read from the last element of the log, see below


## How a move is stored
A move is a 16 bits integer _(`move`, a `uint16_t`)_ :

| Bits | Content |
|---|---|
| 0 to 5 | starting square |
| 6 to 11 | arrival square |
| 12 to 15 | flag |

| Flag | Value |
|---|---:|
| No_Flag | 0 |
| En_Passant_Flag | 1 |
| Castle_Flag | 2 |
| Pawn_Two_Up_Flag | 3 |
| Promote_To_Queen_Flag | 4 |
| Promote_To_Knight_Flag | 5 |
| Promote_To_Rook_Flag | 6 |
| Promote_To_Bishop_Flag | 7 |

* The piece moved and the piece captured aren't in the move : they are read on the board when the move is made
* A castle is the move of the king _(e1 to g1 or c1 for the white one)_ with `Castle_Flag`, the rook is moved by `move_rook_during_castle`
* The move 0 is the null move, tested with `is_move_null` : it means « no move found »
* `get_move_value`, `get_from_square`, `get_to_square` and `get_flag` build and read a move


## The log of the game
The log _(`chess_log`)_ is an array with one element per half-move played, in the game and in the line being searched :

| Field | Content |
|---|---|
| `move_value` | the move |
| `captured_piece_type` | the piece captured by this move, or NOTHING |
| `castling_rights` | the castling rights after this move |

It does three jobs :
* **Undoing a move** : the last element says where the piece came from and which piece has to be put back
* **Castling rights** : 6 bits that say which piece has already moved, so all the bits at 0 means that every castle is still allowed

| Bit | Meaning |
|---:|---|
| 0 | the white king has moved |
| 1 | the white rook of the a file has moved |
| 2 | the white rook of the h file has moved |
| 3 | the black king has moved |
| 4 | the black rook of the a file has moved |
| 5 | the black rook of the h file has moved |

* **En passant** : an en passant capture is possible when the last move of the log has `Pawn_Two_Up_Flag`. There is no en passant square stored anywhere

Other details :
* The log can hold 2048 half-moves _(`max_size_log_array`)_ : the moves of the game plus the ones of the line searched
* `bitboards_log` keeps a copy of the bitboards after each move of the game, for the rule of the 3 repetitions of `is_draw_forced_by_rules`. It's apart from the log because copying 14 bitboards at every node of the search would be too slow : the search detects the repetitions with the Zobrist keys instead
* `log_setup_size` and `log_setup_halfmove_clock` describe the start of a log that comes from a FEN, see below


## Attacks of the pieces
`initialize_attacks` computes once, at the start of the program, the squares attacked from each of the 64 squares :

| Array | Content |
|---|---|
| `precomputed_pawn_attacks[color][square]` | the squares from which a pawn threatens this square |
| `precomputed_pawn_moves[color][square]` | the squares where a pawn can go |
| `precomputed_knight_attacks[square]` | the 8 jumps of a knight, without the ones that leave the board |
| `precomputed_king_attacks[square]` | the squares around a king |
| `precomputed_rook_attacks[square]` | the rank and the file, on an empty board |
| `precomputed_bishop_attacks[square]` | the two diagonals, on an empty board |

* **Knight, king and pawn** : one read in the array, then a `&` with the bitboard of the pieces looked for
* **Rook, bishop and queen** : the pieces in the way have to be taken into account. `calculate_rook_attacks_with_blockers` and `calculate_bishop_attacks_with_blockers` walk in the 4 directions from the square and stop on the first piece met, which is included _(it can be captured)_. The queen uses both. There are no magic bitboards
* **`is_case_threatened`** answers « is this square attacked by the opponent ? » by putting each type of piece on the square and looking if it reaches an opponent piece of the same type : the pawns, the knights and the king first _(the cheapest)_, then the rook lines and the diagonals
* **`is_check`** is `is_case_threatened` on the square of the king


## Move generation
`generate_all_possible_moves` gives an array with all the **legal** moves of the side to play, and `generate_all_possible_moves_that_capture` only gives the captures, for the quiescence search.

1. Going through the squares that hold a piece of the side to play
2. The special moves are looked at first : the en passant capture _(only when the last move of the log is a pawn moved two squares)_ and the castle
3. For the other moves, the arrival squares come from the attacks of the piece _(with the pieces in the way for a pawn, a rook, a bishop and a queen)_, without the squares held by a piece of the same color. They are read one by one from the bitboard with `__builtin_ctzll`
4. **Each move is then played, `is_check` is called on the king of the side that just played, and the move is undone.** The move is kept only if the king isn't in check. There is no computation of the pinned pieces : this test covers the pins, the checks to answer and the en passant that uncovers the king
5. The array has 100 moves _(`max_size_valid_moves_array`)_, and is extended to 218 when it's full _(the maximum possible in a chess position)_

Rules of the special moves :
* **Castle** _(`is_castle_possible`)_ : the king and the rook haven't moved _(castling rights of the log)_, the squares between them are empty, the king isn't in check and the squares it crosses aren't attacked
* **En passant** _(`is_en_passant_possible`)_ : the last move is a pawn moved two squares, and it's next to the pawn that captures. The captured pawn isn't on the arrival square, `clear_piece_during_en_passant` removes it
* **Promotion** _(`is_promotion_possible`)_ : a pawn reaching the last rank. Only the promotion to a queen is generated by default. `are_under_promotions_generated` adds the promotions to a rook, a bishop and a knight, it's used by the perft


## Making and undoing a move
There are two pairs of functions :

| Functions | Used by | What they update |
|---|---|---|
| `make_move_AI` / `undo_last_move_AI` | the search, the UCI program | the bitboards, the log, the color to play |
| `make_move` + `update_log_and_captured_piece_and_score` / `undo_last_move` | a game with the list of the captured pieces | the same, plus the captured pieces and the score |

`make_move_AI` receives a move that is supposed to be legal :
1. Reading on the board the piece that moves and the piece that is captured
2. Adding the element to the log
3. Moving the piece on its bitboard and on the bitboard of its color, removing the captured piece, moving the rook for a castle, removing the pawn for an en passant
4. Replacing the pawn by the piece of the flag for a promotion
5. Updating the castling rights in the log : a right is lost as soon as the king or the rook isn't on its starting square anymore, so a rook that is captured there is covered too
6. Changing the color to play

`undo_last_move_AI` does the reverse from the last element of the log : nothing else is saved, so a position is restored without any copy of the board.


## End of the game
* `get_check_state` gives the state for a color : `NO_CHECK`, `CHECK`, `CHECK_MATE` _(in check and no legal move)_ or `STALEMATE` _(not in check and no legal move)_
* `are_they_possible_moves` stops at the first legal move found, it's faster than generating all of them. It tries every pair of squares with `is_legal_move`, which says if a piece can go from one to the other
* `is_draw_forced_by_rules` gives `DRAW_BY_3_RULES`, `DRAW_BY_50_RULES` or `NO_DRAW`, from the log of the bitboards and from the log
* `is_game_over` and `get_game_result` put these together


## Positions loaded from a FEN
`set_start_position` puts the starting position with an empty log, and `set_position_from_fen` reads a FEN, for example `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`.

Since the castling rights and the en passant only exist in the log, a position loaded from a FEN starts with **one element in the log that isn't a move really played** :
* its `castling_rights` hold the rights of the FEN
* its `move_value` is the pawn move of two squares that allows the en passant, when the FEN gives an en passant square
* `log_setup_size = 1` says that this element must never be undone
* `log_setup_halfmove_clock` holds the number of half-moves of the FEN for the 50 moves rule

The function returns false when the FEN isn't valid _(a rank that isn't complete, an unknown letter, not exactly one king of each color)_, and the board is put in the starting position.


## Checking the move generation
A perft counts all the positions reached at a given depth from a position, and compares the number to the value known for this position. One wrong move anywhere changes the count.

* `./Chess_Engine/UCI/Perft.x` runs 7 reference positions that cover the castlings, the en passant captures, the promotions and the pinned pieces _(28 million positions counted in about one second)_
* `./Chess_Engine/UCI/Perft.x 3 "<fen>"` gives the count for each first move of a position, to find which move is wrong by comparing with another engine
* It has to be run after every change in this folder

This check found the bugs of the move generation listed in the [main README](../../README.md#corrections-made-to-the-engine) : the squares of the castles, the color tested for the black promotions, the en passant to the left.


## Limitations
* The AI only generates the promotions to a queen during its search, the other ones are understood when the opponent plays them
* A position loaded from a FEN doesn't use its last field _(number of the move)_
* The draw by insufficient material isn't taken into account
* The move generation is the slowest part of the engine : every candidate move is played and undone to test the check, the rook and bishop attacks are walked square by square, and an array is allocated at every call
