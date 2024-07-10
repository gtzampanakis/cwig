#include <stdio.h>
#include <stdlib.h>

#define POSITION_MEMORY_SIZE (1024 * 1024)

#define COLOR_WHITE 0b0000
#define COLOR_BLACK 0b1000

#define PIECE_EMPTY     0b000
#define PIECE_PAWN      0b001
#define PIECE_ROOK      0b010
#define PIECE_KNIGHT    0b011
#define PIECE_BISHOP    0b100
#define PIECE_QUEEN     0b101
#define PIECE_KING      0b110

#define P_WHITE ( COLOR_WHITE | PIECE_PAWN )
#define R_WHITE ( COLOR_WHITE | PIECE_ROOK )
#define N_WHITE ( COLOR_WHITE | PIECE_KNIGHT )
#define B_WHITE ( COLOR_WHITE | PIECE_BISHOP )
#define Q_WHITE ( COLOR_WHITE | PIECE_QUEEN )
#define K_WHITE ( COLOR_WHITE | PIECE_KING )
 
#define P_BLACK ( COLOR_BLACK | PIECE_PAWN )
#define R_BLACK ( COLOR_BLACK | PIECE_ROOK )
#define N_BLACK ( COLOR_BLACK | PIECE_KNIGHT )
#define B_BLACK ( COLOR_BLACK | PIECE_BISHOP )
#define Q_BLACK ( COLOR_BLACK | PIECE_QUEEN )
#define K_BLACK ( COLOR_BLACK | PIECE_KING )

#define CASTLING_BLACK_KINGSIDE     0b1000;
#define CASTLING_BLACK_QUEENSIDE    0b0100;
#define CASTLING_WHITE_KINGSIDE     0b0010;
#define CASTLING_WHITE_QUEENSIDE    0b0001;

#define MOVE_LIST_INITIAL_CAPACITY 64

typedef char Piece;
typedef char Castling;
typedef char Direction;
typedef char File;
typedef char Rank;
typedef char Color;
typedef unsigned int Ply;
typedef double Val;

typedef struct {
    File f;
    Rank r;
} Sq;

const Direction DIR_U = 0;
const Direction DIR_R = 1;
const Direction DIR_D = 2;
const Direction DIR_L = 3;
const Direction DIR_UR = 4;
const Direction DIR_DR = 5;
const Direction DIR_DL = 6;
const Direction DIR_UL = 7;
const Direction DIR_NUR = 8;
const Direction DIR_NRU = 9;
const Direction DIR_NRD = 10;
const Direction DIR_NDR = 11;
const Direction DIR_NDL = 12;
const Direction DIR_NLD = 13;
const Direction DIR_NLU = 14;
const Direction DIR_NUL = 15;

Direction ALL_DIRECTIONS[16] = {
    DIR_U, DIR_R, DIR_D, DIR_L,
    DIR_UR, DIR_DR, DIR_DL, DIR_UL,
    DIR_NUR, DIR_NRU, DIR_NRD, DIR_NDR,
    DIR_NDL, DIR_NLD, DIR_NLU, DIR_NUL,
};

typedef struct Move {
    Sq from;
    Sq to;
} Move;

typedef struct MoveList {
    int len;
    int capacity;
    Move *data;
} MoveList;

MoveList make_move_list() {
    Move *data = calloc(MOVE_LIST_INITIAL_CAPACITY, sizeof(Move));
    MoveList move_list = {
        .len = 0,
        .capacity = MOVE_LIST_INITIAL_CAPACITY,
        .data = data
    };
    return move_list;
}

Sq make_sq(File f, Rank r) {
    Sq sq = { .f = f, .r = r };
    return sq;
}

void free_move_list(MoveList *ml) {
    free(ml->data);
}

Move *move_appended_to_move_list(MoveList *ml) {
    if (ml->len == ml->capacity) {
        int new_capacity = ml->capacity + MOVE_LIST_INITIAL_CAPACITY;
        ml->data = reallocarray(ml->data, new_capacity, sizeof(Move)); 
        if (ml->data == NULL) {
            printf("reallocarray returned NULL. Aborting");
            abort();
        }
        ml->capacity = new_capacity;
    }
    return ml->data + ml->len++;
}

void truncate_move_list(MoveList *ml) {
    ml->capacity = ml->len;
    ml->data = reallocarray(ml->data, ml->len, sizeof(Move));
}

typedef struct Pos {
    Piece placement[8][8];
    Color active_color;
    Castling castling;
    Sq en_passant;
    int halfmoves;
    int fullmoves;

    MoveList moves;
} Pos;

Pos *make_position() {
     Pos *p = malloc(sizeof (Pos));
     if (p == NULL) {
         printf("Unable to allocation memory for position. Aborting...");
         abort();
     }
     for (int i = 0; i < 8; i++) {
         for (int j = 0; j < 8; j++) {
             p->placement[i][j] = PIECE_EMPTY;
         }
     }
     p->en_passant.f = 0;
     p->en_passant.r = 0;
     p->castling = 0;
     p->halfmoves = 0;
     p->fullmoves = 0;
     return p;
}

Pos *free_position(Pos* pos) {
    free(pos);
}

void set_piece_at_sq(Pos *pos, Sq sq, Piece piece) {
    pos->placement[sq.f][sq.r] = piece;
}

Piece get_piece_at_sq(Pos *pos, Sq sq) {
    return pos->placement[sq.f][sq.r];
}

File algf_to_f(char algf) {
    return algf - 97;
}

Rank algr_to_r(char algr) {
    return algr - 49;
}

char f_to_algf(File f) {
    return f + 97;
}

char r_to_algr(Rank r) {
    return r + 49;
}

void sq_to_algsq(Sq sq, char *str) {
    str[0] = f_to_algf(sq.f);
    str[1] = r_to_algr(sq.r);
}

void print_sq(Sq sq) {
    char str[2];
    sq_to_algsq(sq, str);
    printf("%c%c\n", str[0], str[1]);
}

Pos *decode_fen(char *fen_string) {
    int state = 0;
    char en_passant_str[2];
    int en_passant_idx = 0;

    Pos *p = make_position();

    int i = 0;
    char c;
    Sq sq = make_sq(0, 7);
    while ((c = fen_string[i++]) != '\0') {
        if (c == ' ') {
            state++;
        } else if (state == 0) {
            if (c == 'r') {
                set_piece_at_sq(p, sq, R_BLACK);
                sq.f += 1;
            } else if (c == 'n') {
                set_piece_at_sq(p, sq, N_BLACK);
                sq.f += 1;
            } else if (c == 'b') {
                set_piece_at_sq(p, sq, B_BLACK);
                sq.f += 1;
            } else if (c == 'q') {
                set_piece_at_sq(p, sq, Q_BLACK);
                sq.f += 1;
            } else if (c == 'k') {
                set_piece_at_sq(p, sq, K_BLACK);
                sq.f += 1;
            } else if (c == 'p') {
                set_piece_at_sq(p, sq, P_BLACK);
                sq.f += 1;
            } else if (c == 'R') {
                set_piece_at_sq(p, sq, R_WHITE);
                sq.f += 1;
            } else if (c == 'N') {
                set_piece_at_sq(p, sq, N_WHITE);
                sq.f += 1;
            } else if (c == 'B') {
                set_piece_at_sq(p, sq, B_WHITE);
                sq.f += 1;
            } else if (c == 'Q') {
                set_piece_at_sq(p, sq, Q_WHITE);
                sq.f += 1;
            } else if (c == 'K') {
                set_piece_at_sq(p, sq, K_WHITE);
                sq.f += 1;
            } else if (c == 'P') {
                set_piece_at_sq(p, sq, P_WHITE);
                sq.f += 1;
            } else if (c == '1') {
                sq.f += 1;
            } else if (c == '2') {
                sq.f += 2;
            } else if (c == '3') {
                sq.f += 3;
            } else if (c == '4') {
                sq.f += 4;
            } else if (c == '5') {
                sq.f += 5;
            } else if (c == '6') {
                sq.f += 6;
            } else if (c == '7') {
                sq.f += 7;
            } else if (c == '8') {
                sq.f += 8;
            } else if (c == '/') {
                sq.f = 0;
                sq.r--;
            }
        } else if (state == 1) {
            if (c == 'w') {
                p->active_color = COLOR_WHITE;
            } else if (c == 'b') {
                p->active_color = COLOR_BLACK;
            }
        } else if (state == 2) {
            if (c == 'K') {
                p->castling |= CASTLING_BLACK_KINGSIDE;
            } else if (c == 'Q') {
                p->castling |= CASTLING_BLACK_QUEENSIDE;
            } else if (c == 'k') {
                p->castling |= CASTLING_WHITE_KINGSIDE;
            } else if (c == 'q') {
                p->castling |= CASTLING_WHITE_QUEENSIDE;
            } else if (c == '-') {
                ;
            }
        } else if (state == 3) {
            en_passant_str[en_passant_idx++] = c;
        } else if (state == 4) {
            if (en_passant_str[0] != '-') {
                p->en_passant.f = algf_to_f(en_passant_str[0]);
                p->en_passant.r = algr_to_r(en_passant_str[1]);
            }
            p->halfmoves *= 10;
            p->halfmoves += (c - 48);
        } else if (state == 5) {
            p->fullmoves *= 10;
            p->fullmoves += (c - 48);
        }
    }

    return p;
}

Color piece_color(Piece piece) {
    return 0b1000 & piece;
}

Piece piece_as_white(Piece piece) {
    return 0b0111 & piece;
}

typedef void (*ApplyDirFn)(Sq*);
void apply_dir_u(Sq *sq) { sq->r += 1; }
void apply_dir_d(Sq *sq) { sq->r -= 1; }
void apply_dir_l(Sq *sq) { sq->f -= 1; }
void apply_dir_r(Sq *sq) { sq->f += 1; }

void apply_dir_ur(Sq *sq) { sq->f += 1; sq->r += 1; };
void apply_dir_ul(Sq *sq) { sq->f -= 1; sq->r += 1; };
void apply_dir_dr(Sq *sq) { sq->f += 1; sq->r -= 1; };
void apply_dir_dl(Sq *sq) { sq->f -= 1; sq->r -= 1; };

void apply_dir_n0(Sq *sq) { sq->f += 2; sq->r += 1; };
void apply_dir_n1(Sq *sq) { sq->f += 1; sq->r += 2; };
void apply_dir_n2(Sq *sq) { sq->f += 2; sq->r -= 1; };
void apply_dir_n3(Sq *sq) { sq->f += 1; sq->r -= 2; };
void apply_dir_n4(Sq *sq) { sq->f -= 2; sq->r += 1; };
void apply_dir_n5(Sq *sq) { sq->f -= 1; sq->r += 2; };
void apply_dir_n6(Sq *sq) { sq->f -= 2; sq->r -= 1; };
void apply_dir_n7(Sq *sq) { sq->f -= 1; sq->r -= 2; };

ApplyDirFn rook_dir_fns[] = {
    apply_dir_u, apply_dir_d, apply_dir_l, apply_dir_r, NULL};
ApplyDirFn bishop_dir_fns[] = {
    apply_dir_ur, apply_dir_ul, apply_dir_dr, apply_dir_dl, NULL};
ApplyDirFn queen_dir_fns[] = {
    apply_dir_u, apply_dir_d, apply_dir_l, apply_dir_r,
    apply_dir_ur, apply_dir_ul, apply_dir_dr, apply_dir_dl, NULL};
ApplyDirFn king_dir_fns[] = {
    apply_dir_u, apply_dir_d, apply_dir_l, apply_dir_r,
    apply_dir_ur, apply_dir_ul, apply_dir_dr, apply_dir_dl, NULL};
ApplyDirFn knight_dir_fns[] = {
    apply_dir_n0, apply_dir_n1, apply_dir_n2, apply_dir_n3,
    apply_dir_n4, apply_dir_n5, apply_dir_n6, apply_dir_n7, NULL};
ApplyDirFn white_pawn_move_to_empty_dir_fns[] = { apply_dir_u, NULL };
ApplyDirFn black_pawn_move_to_empty_dir_fns[] = { apply_dir_d, NULL };
ApplyDirFn white_pawn_capture_dir_fns[] = {
                apply_dir_ur, apply_dir_ul, NULL };
ApplyDirFn black_pawn_capture_dir_fns[] = {
                apply_dir_dr, apply_dir_dl, NULL };

MoveList legal_moves_for_piece(Pos* pos, Sq sq0, Piece piece, MoveList *ml) {
    Color own_color = piece_color(piece);

    ApplyDirFn *dir_fns;
    Piece wp = piece_as_white(piece);
    int move_to_empty_allowed;
    int captures_allowed;
    int max_distance;

    for (int n_pass = 0; n_pass < 2; n_pass++) {
        if (n_pass == 0) {
            if (wp == R_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 1;
                dir_fns = rook_dir_fns;
                max_distance = 9999;
            } else if (wp == B_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 1;
                dir_fns = bishop_dir_fns;
                max_distance = 9999;
            } else if (wp == Q_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 1;
                dir_fns = queen_dir_fns;
                max_distance = 9999;
            } else if (wp == K_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 1;
                dir_fns = king_dir_fns;
                max_distance = 1;
            } else if (wp == N_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 1;
                dir_fns = knight_dir_fns;
                max_distance = 1;
            } else if (wp == P_WHITE) {
                captures_allowed = 0;
                move_to_empty_allowed = 1;
                if (own_color == COLOR_WHITE) {
                    dir_fns = white_pawn_move_to_empty_dir_fns;
                    if (sq0.r == 1) {
                        max_distance = 2;
                    } else {
                        max_distance = 1;
                    }
                } else {
                    dir_fns = black_pawn_move_to_empty_dir_fns;
                    if (sq0.r == 6) {
                        max_distance = 2;
                    } else {
                        max_distance = 1;
                    }
                }
            }
        } else if (n_pass == 1) {
            if (wp == P_WHITE) {
                captures_allowed = 1;
                move_to_empty_allowed = 0;
                max_distance = 1;
                if (own_color == COLOR_WHITE) {
                    dir_fns = white_pawn_capture_dir_fns;
                } else {
                    dir_fns = black_pawn_capture_dir_fns;
                }
            } else {
                break;
            }
        }
        ApplyDirFn dir_fn;
        for (int i = 0; (dir_fn = dir_fns[i]) != NULL; i++) {
            Sq sq = { .f = sq0.f, .r = sq0.r };
            int d = max_distance;
            for (;;) {
                dir_fn(&sq);
                if (sq.f < 0 || sq.f > 7 || sq.r < 0 || sq.r> 7) { break; }
                Piece found = get_piece_at_sq(pos, sq);
                Color found_color = piece_color(found);
                if (found == PIECE_EMPTY) {
                    if (move_to_empty_allowed) {
                        Move *move = move_appended_to_move_list(ml);
                        move->from = sq0;
                        move->to = sq;
                    } else {
                        break;
                    }
                } else if (own_color == found_color) {
                    break;
                } else {
                    if (captures_allowed) {
                        Move *move = move_appended_to_move_list(ml);
                        move->from = sq0;
                        move->to = sq;
                    }
                    break;
                }
                if (--d == 0) { break; }
            }
        }
    }

}

int main() {
    char starting_fen[] =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 123 55";
    char empty_fen[] = "8/8/8/8/8/8/8/8 w - - 0 1";
    char fen_problematic_shows_no_valid_moves[] =
                "8/4k3/3N1N2/4Q3/1B6/8/1K6/8 b - - 0 1";
    char fen[] = "8/4k3/3P1P2/4Q3/1B6/8/1K6/8 w - - 0 1";

    Pos *pos = decode_fen(empty_fen);

    MoveList ml = make_move_list();
    Sq sq = make_sq(4, 6);
    legal_moves_for_piece(pos, sq, P_BLACK, &ml);
    for (int i = 0; i < ml.len; i++) {
        Move move2 = *(ml.data + i);
        print_sq(move2.to);
    }

    free_move_list(&ml);

    free_position(pos);

    printf("Done.\n");
    return 0;
}
