## Folder containing the functions that makes the AI search and play

#### Path tree
```
Alpha_Beta/
├── Data/ # the files used for the opening book, it included all the download games, the opening it created and file to convert the formal log into move_values
│
├── AI # functions to make the AI play on top of all the search testing functions
│
├── Evaluation # functions that are related to the evaluation of the position, as an heuristic used in our search of tree of possibilities
│
├── Move_Ordering # ordering the moves to improve the alpha-beta pruning, by ordering the moves that are most likely to be the best moves first.
│
├── Openings # the generation and management of the opening book
│
├── Pieces_Square_Tables # creating all the piece square tables that are used in the game, to give the preferencial positions of the different pieces (also depending on the game phase)
│
├── Pre_Computed_Data # creating all the pre-computed data that are used in the game, without having to recompute them each time
│
├── Transposition_Tables # creating the transposition table of the game, to store the different positions and their values to avoid recomputing them
│
└── Search # the search algorithms of the AI, to find the best move to play (iterative deepening, alpha-beta pruning, quiescence search, transposition tables, opening book)
```

#### Limitations: 
* I haven't fully implemented the insufficient material rules, but in a simple and basic way, it doesn't take into account if the check can be done or not
* The AI will only promote pawn to queen which is the most common way, even if sometimes it's possible that a player decide to promote it to a knight


#### AI Functionalities:
* A modern **alpha–beta search** engine with practical extensions for real play
    * **Depth-limited negamax** style search with alpha–beta bounds for evaluation values
    * **Depth extensions** selectively extend search depth for tactical/critical moves (captures, checks, promotions, threats)
    * **Iterative deepening** run depth = 1, 2, … until time budget, each iteration primes transposition table for the next deeper search
    * **Quiescence search**, after reaching depth limit, continue searching captures (and checks optionally) until “quiet” position to avoid horizon effect, no strict depth limit on Q-search but use safeguards
* **Transpositions tables** store (hash, depth, score, flag, best_move) to reuse previously computed results
* **Opening book** to start the game as the pro player would, to avoid bad openings and save time
* **Evaluation** function that include: 
    * **Material count**
    * **Piece square tables** that give the preferencial position of the different pieces depending on the state of the game
    * **Pawn structure**: passed pawns, isolated doubled pawns
    * **Pawn shield and king safety**: castling advantages, open files near king
    * **Endgame heuristics**: mop_up_score, king activity, opposition


#### Getting all the openings 
* Getting the unique openings (ex : 6 completes moves = 12 moves) : `sed -E 's/^(([^ ]+ ){11}[^ ]+).*/\1/' Download_Games.txt > All_Openings.txt && sort All_Openings.txt | sed '$!N; /^\(.*\)\n\1$/!P; D' > Opening_in_Games.txt`
* Not needed anymore : supressing the one that contain a choice _(ex Nbxd2, two knights can eat a pawn and we need to tell which knights eat the pawn)_ : `sed '/[a-h][a-h]/d' Opening_in_Games.txt > temp_file.txt && mv temp_file.txt Opening_in_Games.txt && rm -f temp_file.txt`, but now I've been able to changed my opening book loading to consider those moves


#### Limitations : 
* The opening book will only make queens out of the promoted pawn
* Book opening with 12 openings deep (18 already tested), problems loading the file and calculating the value of moves restricted me, I should have tried to change everything to be able to search further without aberration (still possible to play like that but sometimes it will crash or do anything which can be frustrating, this is due to the method of reading the file probably deserves to be reviewed).
* Launching the game will crash the memory, occasionally opening files since the openning book was added, otherwise it won't crash normally during the game.

