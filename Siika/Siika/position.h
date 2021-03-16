#pragma once

#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <array>
#include <vector>

#include "chess.h"
#include "bitboard.h"
#include "util.h"

namespace Chess {

	constexpr int MAX_PLYS = 512;

	typedef i32 Value;
	constexpr Value piecetype_values[PIECETYPE_COUNT] = {
			0, 100, 300, 300, 500, 900, 50000
	};

	class Zobrist {
	public:
		Zobrist();

		u64 black_number;
		std::array<u64, PIECE_COUNT* SQUARE_COUNT> piece_numbers;
		std::array<u64, 16> castling_numbers;
		std::array<u64, FILE_COUNT> ep_file_numbers;
	};

	class Position {
	public:
		static const std::string DEFAULT;

		Position() noexcept;
		Position(const std::string& fen) noexcept;
		void set_default() noexcept;
		void set(const std::string& fen) noexcept;

		std::string fen() const noexcept;

		int turn() const noexcept;
		int opponent() const noexcept;
		int fullmove() const noexcept;
		u64 hash() const noexcept;

		bool is_3x_repeat() const noexcept;

		int en_passant_square() const noexcept;
		int castling_rights() const noexcept;
		int halfmove() const noexcept;

		Value material() const noexcept;
		Value material(int color) const noexcept;
		Value material_diff() const noexcept;

		int piece_on(int square) const noexcept;
		int king_square() const noexcept;
		int king_square(int color) const noexcept;
		void put_piece(int piece, int square) noexcept;
		void take_piece(int square) noexcept;
		void move_piece(int from, int to) noexcept;

		Bitboard empty() const noexcept;
		Bitboard pieces() const noexcept;
		Bitboard pieces(int color) const noexcept;
		Bitboard pieces(int type, int color) const noexcept;

		void do_move(Move move) noexcept;
		void undo_move(Move move) noexcept;

		std::vector<Move> legal_moves(bool only_captures = false) noexcept;

		bool is_in_check() const noexcept;
		bool is_in_check(int color) const noexcept;

	private:
		void reset() noexcept;
		int captured_piece() const noexcept;

		template <int C>
		void pawn_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept;
		void knight_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept;
		void bishop_moves(Bitboard p, Bitboard target, Bitboard empty, std::vector<Move>& moves) const noexcept;
		void rook_moves(Bitboard p, Bitboard target, Bitboard empty, std::vector<Move>& moves) const noexcept;
		void king_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept;
		void castling_moves(std::vector<Move>& moves) const noexcept;

		bool is_legal(Move move) noexcept;
		bool is_square_attacked(int square, int defender) const noexcept;

		u64 calculate_hash() const;
		inline u64 piece_hash(int square) const {
			return zobrist_.piece_numbers[piece_on(square) * SQUARE_COUNT + square];
		}
		
		void update_opponent_attacks() noexcept;

	private:
		int turn_;
		int fullmove_;
		std::array<int, SQUARE_COUNT> squares_;

		std::array<Bitboard, PIECE_COUNT> bitboards_;

		int ply_;
		struct Undo {
			unsigned char ep_;
			unsigned char cr_;
			unsigned char cap_;
			unsigned char halfmove_;
			u64 hash_;
			Bitboard opponent_attacks_;
			int checkers_;
		} undo_[MAX_PLYS];

		static Zobrist zobrist_;

		Value material_[NO_COLOR + 1] = { 0 };
	};

	inline int Position::turn() const noexcept { return turn_; }
	inline int Position::opponent() const noexcept { return color_flip(turn_); }
	inline int Position::fullmove() const noexcept { return fullmove_; }
	inline u64 Position::hash() const noexcept { return undo_[ply_].hash_; }

	inline int Position::en_passant_square() const noexcept { return undo_[ply_].ep_; }
	inline int Position::castling_rights() const noexcept { return undo_[ply_].cr_; }
	inline int Position::halfmove() const noexcept { return undo_[ply_].halfmove_; }
	inline int Position::captured_piece() const noexcept { return undo_[ply_].cap_; }

	inline Value Position::material() const noexcept { return material(turn()); }
	inline Value Position::material(int color) const noexcept { return material_[color]; }
	inline Value Position::material_diff() const noexcept { return material() - material(opponent()); }


	inline int Position::piece_on(int square) const noexcept {
		return squares_[square];
	}

	inline int Position::king_square() const noexcept { return king_square(turn()); }
	inline int Position::king_square(int color) const noexcept { return Bitboards::lsb(pieces(KING, color)); }

	inline void Position::put_piece(int piece, int square) noexcept {
		squares_[square] = piece;

		bitboards_[piece] |= Bitboards::make(square);
		bitboards_[piece_color(piece)] |= Bitboards::make(square);

		// material
		material_[piece_color(piece)] += piecetype_values[piece_type(piece)];
	}

	inline void Position::take_piece(int square) noexcept {
		int piece = squares_[square];
		squares_[square] = NO_PIECE;

		bitboards_[piece] &= ~Bitboards::make(square);
		bitboards_[piece_color(piece)] &= ~Bitboards::make(square);

		// material
		material_[piece_color(piece)] -= piecetype_values[piece_type(piece)];
	}

	inline void Position::move_piece(int from, int to) noexcept {
		int piece = squares_[from];
		squares_[to] = squares_[from];
		squares_[from] = NO_PIECE;

		Bitboard fromto = Bitboards::make(from) | Bitboards::make(to);

		bitboards_[piece] ^= fromto;
		bitboards_[piece_color(piece)] ^= fromto;
	}

	inline Bitboard Position::empty() const noexcept {
		return ~pieces();
	}

	inline Bitboard Position::pieces() const noexcept {
		return bitboards_[WHITE] | bitboards_[BLACK];
	}

	inline Bitboard Position::pieces(int color) const noexcept {
		return bitboards_[color];
	}

	inline Bitboard Position::pieces(int type, int color) const noexcept {
		return bitboards_[make_piece(type, color)];
	}

	template <int C>
	void Position::pawn_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept {
		Bitboard enemy = pieces(opponent());
		Bitboard dest, prom;

		constexpr int Up = C == WHITE ? NORTH : SOUTH;
		constexpr Bitboard Homerank = C == WHITE ? Bitboards::RANK_2 : Bitboards::RANK_7;
		constexpr Bitboard Promrank = C == WHITE ? Bitboards::RANK_8 : Bitboards::RANK_1;

		// single push
		dest = Bitboards::shift<Up>(p) & empty() & target;
		prom = dest & Promrank;
		dest ^= prom;

		while (prom) {
			int to = Bitboards::pop(prom);
			moves.push_back(make_move(to - Up, to, PROMOTION_QUEEN));
			moves.push_back(make_move(to - Up, to, PROMOTION_ROOK));
			moves.push_back(make_move(to - Up, to, PROMOTION_BISHOP));
			moves.push_back(make_move(to - Up, to, PROMOTION_KNIGHT));
		}

		while (dest) {
			int to = Bitboards::pop(dest);
			moves.push_back(make_move(to - Up, to));
		}

		// double push
		dest = Bitboards::shift<Up>(p & Homerank) & empty();
		dest = Bitboards::shift<Up>(dest) & empty() & target;
		while (dest) {
			int to = Bitboards::pop(dest);
			moves.push_back(make_move(to - Up - Up, to, PAWN_DOUBLE_PUSH));
		}

		// caps
		int ep = en_passant_square();
		Bitboard epbb = Bitboards::EMPTY;
		if (ep != NO_SQUARE) { epbb = Bitboards::make(ep); }

		// east
		dest = Bitboards::shift<Up + EAST>(p) & target;
		if (epbb & dest) {
			moves.push_back(make_move(ep - Up - EAST, ep, EN_PASSANT_CAPTURE));
		}

		dest &= enemy;
		prom = dest & Promrank;
		dest ^= prom;

		while (prom) {
			int to = Bitboards::pop(prom);
			moves.push_back(make_move(to - Up - EAST, to, PROMOTION_QUEEN));
			moves.push_back(make_move(to - Up - EAST, to, PROMOTION_ROOK));
			moves.push_back(make_move(to - Up - EAST, to, PROMOTION_BISHOP));
			moves.push_back(make_move(to - Up - EAST, to, PROMOTION_KNIGHT));
		}
		while (dest) {
			int to = Bitboards::pop(dest);
			moves.push_back(make_move(to - Up - EAST, to));
		}

		// west
		dest = Bitboards::shift<Up + WEST>(p) & target;
		if (epbb & dest) {
			moves.push_back(make_move(ep - Up - WEST, ep, EN_PASSANT_CAPTURE));
		}

		dest &= enemy;
		prom = dest & Promrank;
		dest ^= prom;

		while (prom) {
			int to = Bitboards::pop(prom);
			moves.push_back(make_move(to - Up - WEST, to, PROMOTION_QUEEN));
			moves.push_back(make_move(to - Up - WEST, to, PROMOTION_ROOK));
			moves.push_back(make_move(to - Up - WEST, to, PROMOTION_BISHOP));
			moves.push_back(make_move(to - Up - WEST, to, PROMOTION_KNIGHT));
		}
		while (dest) {
			int to = Bitboards::pop(dest);
			moves.push_back(make_move(to - Up - WEST, to));
		}
	}

	inline bool Position::is_in_check() const noexcept {
		return is_in_check(turn());
	}

	inline bool Position::is_in_check(int color) const noexcept {
		return is_square_attacked(king_square(), turn());
	}
}


#endif // POSITION_H
