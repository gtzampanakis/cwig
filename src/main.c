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

typedef struct {
    int f;
    int r;
} Sq;

typedef char Piece;
typedef char Castling;
typedef char Direction;
typedef char File;
typedef char Rank;
typedef char Color;
typedef unsigned int Ply;
typedef double Val;

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
    MoveList move_list = { 0, MOVE_LIST_INITIAL_CAPACITY, data };
    return move_list;
}

Move *get_empty_move_appended_to_move_list(MoveList *ml) {
    if (ml->len == ml->capacity) {
        int new_capacity = ml->capacity + MOVE_LIST_INITIAL_CAPACITY;
        ml->data = reallocarray(ml->data, new_capacity, sizeof(Move)); 
        if (ml->data == NULL) {
            printf("reallocarray returned NULL. Aborting");
            abort();
        }
        ml->capacity = new_capacity;
    }
    ml->len++;
    return ml->data + sizeof(Move);
}

void truncate_move_list(MoveList *ml) {
    ml->capacity = ml->len;
    ml->data = reallocarray(ml->data, ml->len, sizeof(Move));
}

void free_move_list(MoveList *ml) {
    free(ml->data);
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

typedef struct {
    // Number of bits equal to POSITION_MEMORY_SIZE.
    Pos *next;
    Pos data[POSITION_MEMORY_SIZE];
} PosMemory;

Pos *make_position() {
     Pos *p = malloc(sizeof (Pos));
     //printf("%d", p);
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
    pos->placement[sq.f][sq.r];
}

int algf_to_f(char algf) {
    return algf - 10;
}

int algr_to_r(char algr) {
    return algr - 49;
}

Pos *decode_fen(char *fen_string) {
    int state = 0;
    char en_passant_str[2];
    int en_passant_idx = 0;

    Pos *p = make_position();

    int i = 0;
    char c;
    Sq sq = { 0, 7 };
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

MoveList legal_moves_for_rook(
    Pos* pos, const Sq sq0, Piece piece, MoveList *move_list0
) {
    Color own_color = piece_color(piece);

    // up
    Sq sq;
    Move move;
    sq.f = sq0.f;
    sq.r = sq0.r;
    for (;;) {
        sq.f += 1; sq.r -= 1;
        if (sq.f < 0) { break; }
        if (sq.f > 7) { break; }
        if (sq.r < 0) { break; }
        if (sq.r > 7) { break; }
        Piece found = get_piece_at_sq(pos, sq);
        Color found_color = piece_color(found);
        if (found == PIECE_EMPTY) {
        } else if (own_color == found_color) {
            break;
        } else {
            break;
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

    Pos *pos = decode_fen(starting_fen);

    //printf("%d\n", pos->placement[7][7]);
    //printf("%d\n", pos->active_color);
    //printf("%d\n", pos->castling);
    //printf("%d\n", pos->halfmoves);
    //printf("%d\n", pos->fullmoves);

    free_position(pos);

    printf("Done.\n");
    return 0;
}
