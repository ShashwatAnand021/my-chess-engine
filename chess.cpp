#include<bits/stdc++.h>
#include<windows.h> 
#pragma execution_character_set("utf-8")

using namespace std;

// --- Piece Representations and Values ---
// im using emojis to make the board much more user friendly, the underlying implementation uses characters like q for black queen and Q for white queen

unordered_map<char, string> piece_emoji = {
    {'k', "♔"}, {'q', "♕"}, {'r', "♖"}, {'b', "♗"}, {'n', "♘"}, {'p', "♙"},
    {'K', "♚"}, {'Q', "♛"}, {'R', "♜"}, {'B', "♝"}, {'N', "♞"}, {'P', "♟"},
    {'.', "."}
};

// Piece values for the evaluation function
unordered_map<char, int> piece_value = {
    {'P', 100}, {'N', 320}, {'B', 330}, {'R', 500}, {'Q', 900}, {'K', 20000},
    {'p', -100}, {'n', -320}, {'b', -330}, {'r', -500}, {'q', -900}, {'k', -20000},
    {'.', 0}
};

// --- Piece-Square Tables (PSTs) for Evaluation ---
// These tables give positional bonuses/penalties to pieces based on their location.

vector<vector<int>> pawn_table = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5,  5, 10, 25, 25, 10,  5,  5},
    {0,  0,  0, 20, 20,  0,  0,  0},
    {5, -5,-10,  0,  0,-10, -5,  5},
    {5, 10, 10,-20,-20, 10, 10,  5},
    {0,  0,  0,  0,  0,  0,  0,  0}
};
vector<vector<int>> knight_table = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};
vector<vector<int>> bishop_table = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};
vector<vector<int>> rook_table = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};
vector<vector<int>> queen_table = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};
vector<vector<int>> king_middlegame_table = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};
vector<vector<int>> king_endgame_table = {
    {-50,-40,-30,-20,-20,-30,-40,-50},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-50,-30,-30,-30,-30,-30,-30,-50}
};


// --- Game State Variables ---

vector<vector<char>> board(8, vector<char>(8));
bool is_white_turn = true;

// Castling rights
bool cwk = true; // Can white castle kingside?
bool cwq = true; // similarly checking for queenside
bool cbk = true; 
bool cbq = true; 

int en_passant_sq = -1; // En passant target square, -1 if none. (index 0-63)
int halfmove_clock = 0; // For 50-move rule


// --- Move Structure ---
// Holds all information needed to make and undo a move.
struct Move {
    int from_row, from_col;
    int to_row, to_col;

    char moved_piece = '.';
    char captured_piece = '.';

    bool is_promotion = false;
    char promoted_to = 'Q'; // Default promotion to Queen

    bool is_en_passant = false;
    bool is_castling = false;

    // Store game state before the move to allow for easy undoing
    bool prev_cwk, prev_cwq, prev_cbk, prev_cbq;
    int prev_en_passant_sq;
    int prev_halfmove_clock;
};


// --- Function Declarations ---
// Forward declarations are good practice in C++.
void make_move(Move& m);
void undo_move(Move& m);
bool is_square_attacked(int row, int col, bool by_white);
vector<Move> generate_legal_moves();


// --- Board Setup and Display ---

void make_board() {
    for (int j = 2; j < 6; j++) {
        for (int i = 0; i < 8; i++) {
            board[j][i] = '.';
        }
    }

    board[0] = {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'};
    board[1] = vector<char>(8, 'p');
    board[6] = vector<char>(8, 'P');
    board[7] = {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'};
}

void print_board(bool white_pov = true) {
    cout << "\n     a b c d e f g h\n";
    cout << "   +-----------------+ \n";

    if (white_pov) {
        for (int r = 0; r < 8; ++r) {
            cout << 8 - r << "  | ";
            for (int c = 0; c < 8; ++c) {
                cout << piece_emoji[board[r][c]] << " ";
            }
            cout << "| " << 8 - r << "\n";
        }
    } else { // Black's point of view
        for (int r = 7; r >= 0; --r) {
            cout << 8 - r << "  | ";
            for (int c = 7; c >= 0; --c) {
                cout << piece_emoji[board[r][c]] << " ";
            }
            cout << "| " << 8 - r << "\n";
        }
    }

    cout << "   +-----------------+ \n";
    cout << "     a b c d e f g h\n\n";
}


// --- Move Conversion Utilities ---

int file_to_col(char file) { return file - 'a'; }
int rank_to_row(char rank) { return 8 - (rank - '0'); }

// converts user input (e.g., "e2e4") into a Move structure
Move make_struct_move(string input_move) {
    Move m;
    if (input_move.length() < 4 || input_move.length() > 5) {
        m.from_row = -1; // Invalid move indicator
        return m;
    }

    m.from_col = file_to_col(input_move[0]);
    m.from_row = rank_to_row(input_move[1]);
    m.to_col = file_to_col(input_move[2]);
    m.to_row = rank_to_row(input_move[3]);

    m.moved_piece = board[m.from_row][m.from_col];
    m.captured_piece = board[m.to_row][m.to_col];

    if (input_move.length() == 5) {
        m.is_promotion = true;
        m.promoted_to = toupper(input_move[4]);
    }

    return m;
}


// --- Core Move Logic ---

void make_move(Move& m) {
    // Store previous state for undo_move
    m.prev_cwk = cwk;
    m.prev_cwq = cwq;
    m.prev_cbk = cbk;
    m.prev_cbq = cbq;
    m.prev_en_passant_sq = en_passant_sq;
    m.prev_halfmove_clock = halfmove_clock;

    // Reset 50-move clock if pawn move or capture
    if (tolower(m.moved_piece) == 'p' || m.captured_piece != '.') {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    // Move the piece
    board[m.from_row][m.from_col] = '.';
    if (m.is_promotion) {
        board[m.to_row][m.to_col] = is_white_turn ? m.promoted_to : tolower(m.promoted_to);
    } else {
        board[m.to_row][m.to_col] = m.moved_piece;
    }

    // Handle En Passant
    en_passant_sq = -1; // Reset en passant square
    if (m.is_en_passant) {
        if (is_white_turn) {
            board[m.to_row + 1][m.to_col] = '.';
        } else {
            board[m.to_row - 1][m.to_col] = '.';
        }
    } else if (tolower(m.moved_piece) == 'p' && abs(m.to_row - m.from_row) == 2) {
        // Set new en passant square if a pawn moved two steps
        en_passant_sq = m.from_col + ((m.to_row + m.from_row) / 2) * 8;
    }

    // Handle Castling
    if (m.is_castling) {
        if (m.to_col == 6) { // Kingside
            board[m.to_row][5] = board[m.to_row][7];
            board[m.to_row][7] = '.';
        } else if (m.to_col == 2) { // Queenside
            board[m.to_row][3] = board[m.to_row][0];
            board[m.to_row][0] = '.';
        }
    }

    // Update castling rights on king or rook moves
    if (m.moved_piece == 'K') { cwk = cwq = false; }
    if (m.moved_piece == 'k') { cbk = cbq = false; }
    if (m.moved_piece == 'R' && m.from_row == 7 && m.from_col == 0) { cwq = false; }
    if (m.moved_piece == 'R' && m.from_row == 7 && m.from_col == 7) { cwk = false; }
    if (m.moved_piece == 'r' && m.from_row == 0 && m.from_col == 0) { cbq = false; }
    if (m.moved_piece == 'r' && m.from_row == 0 && m.from_col == 7) { cbk = false; }

    // Update castling rights if a rook is captured
    if (m.captured_piece == 'R') {
        if (m.to_row == 7 && m.to_col == 0) cwq = false;
        if (m.to_row == 7 && m.to_col == 7) cwk = false;
    }
    if (m.captured_piece == 'r') {
        if (m.to_row == 0 && m.to_col == 0) cbq = false;
        if (m.to_row == 0 && m.to_col == 7) cbk = false;
    }

    // IMPORTANT: Flip turn at the end of a successful move.
    is_white_turn = !is_white_turn;
}

void undo_move(Move& m) {
    // IMPORTANT: Flip turn back first.
    is_white_turn = !is_white_turn;

    // Restore board state
    char piece_to_restore = m.is_promotion ? (is_white_turn ? 'P' : 'p') : m.moved_piece;
    board[m.from_row][m.from_col] = piece_to_restore;
    board[m.to_row][m.to_col] = m.captured_piece; // May restore '.'

    // Undo en passant
    if (m.is_en_passant) {
        board[m.to_row][m.to_col] = '.';
        if (is_white_turn) {
            board[m.to_row + 1][m.to_col] = 'p';
        } else {
            board[m.to_row - 1][m.to_col] = 'P';
        }
    }

    // Undo castling
    if (m.is_castling) {
        if (m.to_col == 6) { // Kingside
            board[m.to_row][7] = board[m.to_row][5];
            board[m.to_row][5] = '.';
        } else if (m.to_col == 2) { // Queenside
            board[m.to_row][0] = board[m.to_row][3];
            board[m.to_row][3] = '.';
        }
    }

    // Restore game state variables from the move object
    cwk = m.prev_cwk;
    cwq = m.prev_cwq;
    cbk = m.prev_cbk;
    cbq = m.prev_cbq;
    en_passant_sq = m.prev_en_passant_sq;
    halfmove_clock = m.prev_halfmove_clock;
}


// --- Move Generation ---

void add_move(int r1, int c1, int r2, int c2, vector<Move>& moves, bool is_castling = false, bool is_promo = false, char promo_piece = ' ', bool is_ep = false) {
    Move m;
    m.from_row = r1; m.from_col = c1;
    m.to_row = r2; m.to_col = c2;
    m.moved_piece = board[r1][c1];
    m.captured_piece = board[r2][c2];
    m.is_castling = is_castling;
    m.is_promotion = is_promo;
    m.promoted_to = promo_piece;
    m.is_en_passant = is_ep;
    if(is_ep) {
       m.captured_piece = isupper(m.moved_piece) ? 'p' : 'P';
    }
    moves.push_back(m);
}

// Sliders  - they cannot hop over pieces like the knight (Rook, Bishop, Queen)
void generate_slider_moves(int r, int c, const vector<pair<int, int>>& dirs, vector<Move>& moves) {
    char piece = board[r][c];
    for (auto [dr, dc] : dirs) {
        int nr = r + dr, nc = c + dc;
        while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            char target = board[nr][nc];
            if (target == '.') {
                add_move(r, c, nr, nc, moves);
            } else {
                if (isupper(piece) != isupper(target)) {
                    add_move(r, c, nr, nc, moves); // Capture
                }
                break; // Path is blocked
            }
            nr += dr;
            nc += dc;
        }
    }
}

void generate_pseudo_legal_moves(vector<Move>& moves) {
    moves.clear();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece == '.' || (is_white_turn != isupper(piece))) continue;

            char lower_piece = tolower(piece);

            if (lower_piece == 'p') {
                int dir = is_white_turn ? -1 : 1;
                int start_rank = is_white_turn ? 6 : 1;
                int promo_rank = is_white_turn ? 0 : 7;

                // 1-step forward
                if (r + dir >= 0 && r + dir < 8 && board[r + dir][c] == '.') {
                    if (r + dir == promo_rank) {
                        for (char p : {'Q', 'R', 'B', 'N'}) add_move(r, c, r + dir, c, moves, false, true, p);
                    } else {
                        add_move(r, c, r + dir, c, moves);
                    }
                    // 2-step forward
                    if (r == start_rank && board[r + 2 * dir][c] == '.') {
                        add_move(r, c, r + 2 * dir, c, moves);
                    }
                }
                // Captures
                for (int dc : {-1, 1}) {
                    if (c + dc >= 0 && c + dc < 8) {
                        // Regular capture
                        if (r + dir >= 0 && r + dir < 8 && board[r + dir][c + dc] != '.' && isupper(board[r + dir][c + dc]) != is_white_turn) {
                            if (r + dir == promo_rank) {
                                for (char p : {'Q', 'R', 'B', 'N'}) add_move(r, c, r + dir, c + dc, moves, false, true, p);
                            } else {
                                add_move(r, c, r + dir, c + dc, moves);
                            }
                        }
                        // En passant
                        if (en_passant_sq != -1 && (r + dir) * 8 + (c + dc) == en_passant_sq) {
                             add_move(r, c, r + dir, c + dc, moves, false, false, ' ', true);
                        }
                    }
                }
            } else if (lower_piece == 'n') {
                vector<pair<int,int>> jumps = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
                for (auto [dr, dc] : jumps) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                        if (board[nr][nc] == '.' || isupper(board[nr][nc]) != is_white_turn) {
                           add_move(r, c, nr, nc, moves);
                        }
                    }
                }
            } else if (lower_piece == 'b') {
                generate_slider_moves(r, c, {{-1,-1},{-1,1},{1,-1},{1,1}}, moves);
            } else if (lower_piece == 'r') {
                generate_slider_moves(r, c, {{-1,0},{1,0},{0,-1},{0,1}}, moves);
            } else if (lower_piece == 'q') {
                generate_slider_moves(r, c, {{-1,-1},{-1,1},{1,-1},{1,1},{-1,0},{1,0},{0,-1},{0,1}}, moves);
            } else if (lower_piece == 'k') {
                // King moves
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (dr == 0 && dc == 0) continue;
                        int nr = r + dr, nc = c + dc;
                        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                           if (board[nr][nc] == '.' || isupper(board[nr][nc]) != is_white_turn) {
                                add_move(r, c, nr, nc, moves);
                            }
                        }
                    }
                }
                // Castling, checking all the constraingts
                if (is_white_turn) {
                    if (cwk && board[7][5]=='.' && board[7][6]=='.' && !is_square_attacked(7,4,false) && !is_square_attacked(7,5,false) && !is_square_attacked(7,6,false)) add_move(7,4,7,6,moves,true);
                    if (cwq && board[7][3]=='.' && board[7][2]=='.' && board[7][1]=='.' && !is_square_attacked(7,4,false) && !is_square_attacked(7,3,false) && !is_square_attacked(7,2,false)) add_move(7,4,7,2,moves,true);
                } else {
                    if (cbk && board[0][5]=='.' && board[0][6]=='.' && !is_square_attacked(0,4,true) && !is_square_attacked(0,5,true) && !is_square_attacked(0,6,true)) add_move(0,4,0,6,moves,true);
                    if (cbq && board[0][3]=='.' && board[0][2]=='.' && board[0][1]=='.' && !is_square_attacked(0,4,true) && !is_square_attacked(0,3,true) && !is_square_attacked(0,2,true)) add_move(0,4,0,2,moves,true);
                }
            }
        }
    }
}

// finds the king and returns true if it's in check.
bool is_in_check(bool check_white) {
    int king_r = -1, king_c = -1;
    char king_char = check_white ? 'K' : 'k';
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board[r][c] == king_char) {
                king_r = r;
                king_c = c;
                break;
            }
        }
    }
    return is_square_attacked(king_r, king_c, !check_white);
}

vector<Move> generate_legal_moves() {
    vector<Move> pseudo_moves;
    vector<Move> legal_moves;
    generate_pseudo_legal_moves(pseudo_moves);

    for (auto& m : pseudo_moves) {
        make_move(m);
        // The move is legal if the king of the player who just moved is NOT in check
        if (!is_in_check(!is_white_turn)) {
            legal_moves.push_back(m);
        }
        undo_move(m);
    }
    return legal_moves;
}


// ---  Evaluation ---

int count_material() {
    int total = 0;
    for(int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            total += piece_value[board[r][c]];
        }
    }
    return total;
}

int evaluate_position() {
    int score = 0;
    int material = 0;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece == '.') continue;
            
            material += piece_value[piece];
            
            int positional_score = 0;
            int eval_r = isupper(piece) ? r : 7 - r;
            int eval_c = c;

            switch (tolower(piece)) {
                case 'p': positional_score = pawn_table[eval_r][eval_c]; break;
                case 'n': positional_score = knight_table[eval_r][eval_c]; break;
                case 'b': positional_score = bishop_table[eval_r][eval_c]; break;
                case 'r': positional_score = rook_table[eval_r][eval_c]; break;
                case 'q': positional_score = queen_table[eval_r][eval_c]; break;
                case 'k': {
                    // Endgame evaluation for king position is different
                    bool is_endgame = abs(material) < 1500; // Heuristic for endgame, king in the center of the board is favorable in the endgame
                    if (is_endgame) {
                        positional_score = king_endgame_table[eval_r][eval_c];
                    } else {
                        positional_score = king_middlegame_table[eval_r][eval_c];
                    }
                    break;
                }
            }
            score += isupper(piece) ? positional_score : -positional_score;
        }
    }
    
    // The final score is material + positional score.
    return material + score;
}

int minimax(int depth, int alpha, int beta) {
    if (depth == 0) {
        // Return score relative to the current player
        return evaluate_position() * (is_white_turn ? 1 : -1);
    }

    vector<Move> legal_moves = generate_legal_moves();
    if (legal_moves.empty()) {
        if (is_in_check(is_white_turn)) return -100000; // Checkmate (worst possible score)
        return 0; // Stalemate
    }

    int best_eval = -1e9;
    for (auto& m : legal_moves) {
        make_move(m);
        int eval = -minimax(depth - 1, -beta, -alpha);
        undo_move(m);
        
        if (eval > best_eval) {
            best_eval = eval;
        }
        alpha = max(alpha, eval);
        if (alpha >= beta) {
            break; // Pruning
        }
    }
    return best_eval;
}

Move get_best_move(int depth) {
    vector<Move> legal_moves = generate_legal_moves();
    Move best_move;
    int best_score = -1e9; // starting with a very low score

    for (auto& m : legal_moves) {
        make_move(m);
        int eval = -minimax(depth - 1, -1e9, 1e9);
        undo_move(m);

        if (eval > best_score) {
            best_score = eval;
            best_move = m;
        }
    }
    return best_move;
}

// checks if a square is attacked by a specified side
bool is_square_attacked(int row, int col, bool by_white) {
    char p, n, b, r, q, k;
    if (by_white) { p = 'P'; n = 'N'; b = 'B'; r = 'R'; q = 'Q'; k = 'K'; }
    else { p = 'p'; n = 'n'; b = 'b'; r = 'r'; q = 'q'; k = 'k'; }

    // Pawns
    int dir = by_white ? 1 : -1;
    if (row + dir >= 0 && row + dir < 8) {
        if (col - 1 >= 0 && board[row + dir][col - 1] == p) return true;
        if (col + 1 < 8 && board[row + dir][col + 1] == p) return true;
    }
    // Knights
    vector<pair<int,int>> jumps = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(auto [dr, dc] : jumps) if(row+dr>=0 && row+dr<8 && col+dc>=0 && col+dc<8 && board[row+dr][col+dc]==n) return true;
    // King
    for(int dr=-1; dr<=1; ++dr) for(int dc=-1; dc<=1; ++dc) if(row+dr>=0 && row+dr<8 && col+dc>=0 && col+dc<8 && board[row+dr][col+dc]==k) return true;
    
    // Sliders (Rook, Bishop, Queen)
    vector<pair<int,int>> diag = {{-1,-1},{-1,1},{1,-1},{1,1}};
    vector<pair<int,int>> straight = {{-1,0},{1,0},{0,-1},{0,1}};
    for(auto [dr,dc]:diag) for(int i=1;i<8;++i) { int nr=row+i*dr, nc=col+i*dc; if(nr<0||nr>=8||nc<0||nc>=8) break; char target=board[nr][nc]; if(target!='.'){ if(target==b||target==q) return true; break; }}
    for(auto [dr,dc]:straight) for(int i=1;i<8;++i) { int nr=row+i*dr, nc=col+i*dc; if(nr<0||nr>=8||nc<0||nc>=8) break; char target=board[nr][nc]; if(target!='.'){ if(target==r||target==q) return true; break; }}

    return false;
}

// --- Game Draw Conditions ---

bool is_insufficient_material() {
    vector<char> pieces;
    for (const auto& row : board) {
        for (char p : row) {
            if (p != '.' && tolower(p) != 'k') {
                pieces.push_back(p);
            }
        }
    }
    if (pieces.empty()) return true; // King vs King
    if (pieces.size() == 1 && (tolower(pieces[0]) == 'b' || tolower(pieces[0]) == 'n')) return true; // King vs King + Knight/Bishop
    
    // King + 2 Knights vs King
    if (pieces.size() == 2 && tolower(pieces[0]) == 'n' && tolower(pieces[1]) == 'n') {
        bool is_white_knight1 = isupper(pieces[0]);
        bool is_white_knight2 = isupper(pieces[1]);
        if (is_white_knight1 == is_white_knight2) return true;
    }
    return false;
}

// --- Main Game Loop ---

int main() {
    SetConsoleOutputCP(CP_UTF8);
    make_board();

    cout << "Enter the move as 'e2e4', where 'e2' is the current piece position and 'e4' is the destination square position.\nIn case of pawn promotion, enter something like e7e8q (the last character denotes the piece to be promoted to, it is queen in this case).\nGOOD LUCK!\n";

    while (true) {
        print_board(is_white_turn);
        vector<Move> legal_moves = generate_legal_moves();

        if (legal_moves.empty()) {
            if (is_in_check(is_white_turn)) {
                cout << (is_white_turn ? "BLACK" : "WHITE") << " WON BY CHECKMATE!\n";
            } else {
                cout << "STALEMATE! It's a draw.\n";
            }
            break;
        }
        if (halfmove_clock >= 100) {
            cout << "DRAW by 50-move rule.\n";
            break;
        }
        if (is_insufficient_material()) {
            cout << "DRAW due to insufficient material.\n";
            break;
        }

        Move current_move;
        if (is_white_turn) {
            cout << "Your move (e.g. e2e4 or e7e8q): ";
            string input;
            cin >> input;

            Move player_move_struct = make_struct_move(input);
            bool valid_move_found = false;
            for (auto& m : legal_moves) {
                if (m.from_row == player_move_struct.from_row && m.from_col == player_move_struct.from_col &&
                    m.to_row == player_move_struct.to_row && m.to_col == player_move_struct.to_col &&
                    (!m.is_promotion || (m.is_promotion && tolower(m.promoted_to) == tolower(player_move_struct.promoted_to)))) {
                    
                    current_move = m; 
                    valid_move_found = true;
                    break;
                }
            }

            if (!valid_move_found) {
                cout << "Invalid move. Try again.\n";
                continue;
            }
        } else {
            cout << "Engine is thinking...\n";
            //for a more challenging game increase depth to 4, 5, or 6(6 will require enormous patience!)
            // to change the depth replace the next line

            // at a depth of 5, it plays at around 1600 elo points!(beats stockfish level 4 on lichess) 
            current_move = get_best_move(5); // Depth can be increased for a stronger engine, do not go beyond 6(at depth 6, it can take anywhere between a few seconds to 6 mins to generate a move) 
            cout << "Engine played: "
                 << char('a' + current_move.from_col) << 8 - current_move.from_row
                 << char('a' + current_move.to_col) << 8 - current_move.to_row;
            if (current_move.is_promotion) cout << char(tolower(current_move.promoted_to));
            cout << "\n";
        }

        make_move(current_move);
        // 'make_move' function already flips the current player.
    }

    cout << "Game over. Thank you for playing!\n";
    return 0;
}
