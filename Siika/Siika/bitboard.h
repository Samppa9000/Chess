#pragma once

#ifndef BITBOARD_H
#define BITBOARD_H

#include "chess.h"

#include <array>

namespace Chess {

	typedef unsigned long long Bitboard;

	class Bitboards {
	public:

		enum : Bitboard {
			ONE = 1,
			EMPTY = 0,
			ALL = ~EMPTY,

			RANK_1 = 0xFF,
			RANK_2 = RANK_1 << 8,
			RANK_3 = RANK_1 << 16,
			RANK_4 = RANK_1 << 24,
			RANK_5 = RANK_1 << 32,
			RANK_6 = RANK_1 << 40,
			RANK_7 = RANK_1 << 48,
			RANK_8 = RANK_1 << 56,

			FILE_A = 0x0101010101010101,
			FILE_B = FILE_A << 1,
			FILE_C = FILE_A << 2,
			FILE_D = FILE_A << 3,
			FILE_E = FILE_A << 4,
			FILE_F = FILE_A << 5,
			FILE_G = FILE_A << 6,
			FILE_H = FILE_A << 7,
		};

		static Bitboard make(int square);

		static int lsb(Bitboard b);
		static int pop(Bitboard& b);
		static int popcount(Bitboard b);

		template <int D>
		static Bitboard shift(Bitboard b);

		template <int D>
		static Bitboard ray(Bitboard b, Bitboard empty);

		template <int Up>
		static Bitboard pawn_attacks(Bitboard b);
		static Bitboard knight_attacks(Bitboard b);
		static Bitboard knight_attacks(int square);
		static Bitboard bishop_attacks(Bitboard b, Bitboard empty);
		static Bitboard rook_attacks(Bitboard b, Bitboard empty);
		static Bitboard king_attacks(Bitboard b);
		static Bitboard king_attacks(int square);

		Bitboards() = delete;

	private: // methods
		static bool initialize();

	private:
		static bool initialized_;
		static std::array<Bitboard, SQUARE_COUNT> knight_attacks_;
		static std::array<Bitboard, SQUARE_COUNT> king_attacks_;
	};

	inline Bitboard Bitboards::make(int square) {
		return ONE << square;
	}

	constexpr int index64[64] = {
		0,  1, 48,  2, 57, 49, 28,  3,
		61, 58, 50, 42, 38, 29, 17,  4,
		62, 55, 59, 36, 53, 51, 43, 22,
		45, 39, 33, 30, 24, 18, 12,  5,
		63, 47, 56, 27, 60, 41, 37, 16,
		54, 35, 52, 21, 44, 32, 23, 11,
		46, 26, 40, 15, 34, 20, 31, 10,
		25, 14, 19,  9, 13,  8,  7,  6
	};

	inline int Bitboards::lsb(Bitboard b) {
		constexpr Bitboard debruijn64 = 0x03f79d71b4cb0a89;
		auto bb = static_cast<i64>(b);
		Bitboard lsb = static_cast<Bitboard>(bb & -bb);
		return index64[(lsb * debruijn64) >> 58];
	}

	inline int Bitboards::pop(Bitboard& b) {
		constexpr Bitboard debruijn64 = 0x03f79d71b4cb0a89;
		auto bb = static_cast<i64>(b);
		Bitboard lsb = static_cast<Bitboard>(bb & -bb);

		auto s = index64[(lsb * debruijn64) >> 58];
		b ^= lsb;
		return s;
	}

	inline int Bitboards::popcount(Bitboard b) {
		int n = 0;
		while (b) {
			pop(b);
			++n;
		}
		return n;
	}

	template <int D>
	Bitboard Bitboards::shift(Bitboard b) {
		if (D == NORTH) { return b << 8; }
		else if (D == SOUTH) { return b >> 8; }
		else if (D == NORTHEAST) { return (b << 9) & ~FILE_A; }
		else if (D == EAST) { return (b << 1) & ~FILE_A; }
		else if (D == SOUTHEAST) { return (b >> 7) & ~FILE_A; }
		else if (D == NORTHWEST) { return (b << 7) & ~FILE_H; }
		else if (D == WEST) { return (b >> 1) & ~FILE_H; }
		else if (D == SOUTHWEST) { return (b >> 9) & ~FILE_H; }
		else { return 0; }
	}

	template <int D>
	Bitboard Bitboards::ray(Bitboard b, Bitboard empty) {
		Bitboard result = EMPTY;

		while (b) {
			b = shift<D>(b);
			result |= b;
			b &= empty;
		}

		return result;
	}

	template <int C>
	Bitboard Bitboards::pawn_attacks(Bitboard b) {
		constexpr int Up = C == WHITE ? NORTH : SOUTH;
		return shift<Up + EAST>(b) | shift<Up + WEST>(b);
	}

	inline Bitboard Bitboards::knight_attacks(int square) {
		return knight_attacks_[square];
	}

	inline Bitboard Bitboards::king_attacks(int square) {
		return king_attacks_[square];
	}

}

#endif // BITBOARD_H
