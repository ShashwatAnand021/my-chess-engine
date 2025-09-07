This is a terminal-based chess engine written in C++.

It FEATURES:
1. Pseudo moves generation
2. Filtering the legal moves
3. All the rules of the game, including castling, en passant, pawn promotion, stalemate, etc.
4. Minimax algorithm with alpha-beta pruning
5. Uses standard piece evaluation and piece-square tables for position evaluation

Here's HOW TO USE it:

compile as "g++ chess.cpp -o chess"
run as "./chess"

g++ chess.cpp -o chess
./chess

Other INSTRUCTIONS:

Enter a move like e2e4, the first two characters corresponding to the piece's current position and the next two to the destination square.
In case of promotion, add an extra character in the end to tell the piece to be promoted to as "e7e8q", this promotes the pawn to a queen (q->queen, r->rook, n->knight, b->bishop)

You can adjust the engine strength by changing the line 691 (Move best_move = get_best_move(5);) - change 5 to the desired value
! For a depth of 5+, the engine is very slow. I am currently working on improving efficiency (ID algorithms with time bounds), but for now, try at most till depth 6.
For depth 6, the engine takes anywhere between a few seconds to ~6 minutes
At a depth of 5, the average move time is ~8s during the middlegame, and it's instantaneous in the endgame!

Good luck for the game!
