#pragma once

#ifndef CHESS_H
#define CHESS_H

#include "util.h"

namespace Chess {

#define AUTHOR "S.Hughes"
#define ENGINE "Siika"
#define MAJOR_VERSION 0
#define MINOR_VERSION 1

	enum Color { WHITE = 0, BLACK = 8, NO_COLOR = 16 };

	enum Rank { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_COUNT };

	enum File { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_COUNT };

	enum Square {
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		SQUARE_COUNT, NO_SQUARE
	};

	enum Direction {
		NORTH = 8, EAST = 1,
		SOUTH = -NORTH, WEST = -EAST,
		NORTHEAST = NORTH + EAST, SOUTHEAST = SOUTH + EAST,
		SOUTHWEST = SOUTH + WEST, NORTHWEST = NORTH + WEST
	};

	enum PieceType {
		NO_PIECETYPE,
		PAWN,
		KNIGHT,
		BISHOP,
		ROOK,
		QUEEN,
		KING,
		PIECETYPE_COUNT
	};

	enum Piece {
		WHITE_PAWN = WHITE + PAWN,
		WHITE_KNIGHT,
		WHITE_BISHOP,
		WHITE_ROOK,
		WHITE_QUEEN,
		WHITE_KING,
		BLACK_PAWN = BLACK + PAWN,
		BLACK_KNIGHT,
		BLACK_BISHOP,
		BLACK_ROOK,
		BLACK_QUEEN,
		BLACK_KING,
		NO_PIECE = NO_PIECETYPE + NO_COLOR,
		PIECE_COUNT
	};

	enum CastlingRights {
		NO_CASTLINGS,
		WHITE_KINGSIDE = 1,
		WHITE_QUEENSIDE = 2,
		BLACK_KINGSIDE = 4,
		BLACK_QUEENSIDE = 8,
		ALL_CASTLINGS = 15
	};

	typedef unsigned short Move;
	constexpr Move NULLMOVE = 0;

	enum MoveFlags : unsigned short {
		NORMAL_MOVE = 0x0,
		PAWN_DOUBLE_PUSH = 0x2000,
		KINGSIDE_CASTLE = 0x4000,
		QUEENSIDE_CASTLE = 0x6000,
		EN_PASSANT_CAPTURE = 0x8000,
		PROMOTION = 0x1000,
		PROMOTION_KNIGHT = 0x5000,
		PROMOTION_BISHOP = 0x7000,
		PROMOTION_ROOK = 0x9000,
		PROMOTION_QUEEN = 0xb000
	};

	constexpr int color_flip(int color) { return color ^ BLACK; }

	constexpr int make_square(int rank, int file) { return (rank << 3) + file; }
	constexpr int square_rank(int square) { return square >> 3; }
	constexpr int square_file(int square) { return square & 7; }

	constexpr bool square_ok(int square) { return square >= A1 && square <= H8; }

	constexpr int pawn_up(int color) { return color == WHITE ? NORTH : SOUTH; }

	constexpr int make_piece(int type, int color) { return type + color; }
	constexpr int piece_type(int piece) { return piece & 7; }
	constexpr int piece_color(int piece) { return piece & (BLACK + NO_COLOR); }

	constexpr Move make_move(int from, int to) { return from + (to << 6); }
	constexpr Move make_move(int from, int to, int flags) { return from + (to << 6) + flags; }
	constexpr int move_from(Move move) { return move & 63; }
	constexpr int move_to(Move move) { return (move >> 6) & 63; }
	constexpr int move_flags(Move move) { return move & 0xF000; }
	constexpr int is_promotion(Move move) { return move & PROMOTION; }
	constexpr int promotion_type(Move move) { return move >> 13; }
}

#endif // CHESS_H