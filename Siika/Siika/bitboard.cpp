#include "bitboard.h"

namespace Chess {

	std::array<Bitboard, SQUARE_COUNT> Bitboards::knight_attacks_;
	std::array<Bitboard, SQUARE_COUNT> Bitboards::king_attacks_;
	bool Bitboards::initialized_ = Bitboards::initialize();

	bool Bitboards::initialize() {
		for (int i = 0; i < 64; i++) {
			auto b = make(i);

			// knight
			knight_attacks_[i] = EMPTY;
			knight_attacks_[i] |= shift<NORTH>(shift<NORTHEAST>(b));
			knight_attacks_[i] |= shift<EAST>(shift<NORTHEAST>(b));
			knight_attacks_[i] |= shift<EAST>(shift<SOUTHEAST>(b));
			knight_attacks_[i] |= shift<SOUTH>(shift<SOUTHEAST>(b));
			knight_attacks_[i] |= shift<SOUTH>(shift<SOUTHWEST>(b));
			knight_attacks_[i] |= shift<WEST>(shift<SOUTHWEST>(b));
			knight_attacks_[i] |= shift<WEST>(shift<NORTHWEST>(b));
			knight_attacks_[i] |= shift<NORTH>(shift<NORTHWEST>(b));

			// king
			king_attacks_[i] = EMPTY;
			king_attacks_[i] |= shift<NORTH>(b);
			king_attacks_[i] |= shift<NORTHEAST>(b);
			king_attacks_[i] |= shift<EAST>(b);
			king_attacks_[i] |= shift<SOUTHEAST>(b);
			king_attacks_[i] |= shift<SOUTH>(b);
			king_attacks_[i] |= shift<SOUTHWEST>(b);
			king_attacks_[i] |= shift<WEST>(b);
			king_attacks_[i] |= shift<NORTHWEST>(b);
		}


		return true;
	}

	Bitboard Bitboards::knight_attacks(Bitboard b) {
		Bitboard result = EMPTY;

		result |= shift<NORTH>(shift<NORTHEAST>(b));
		result |= shift<EAST>(shift<NORTHEAST>(b));
		result |= shift<EAST>(shift<SOUTHEAST>(b));
		result |= shift<SOUTH>(shift<SOUTHEAST>(b));
		result |= shift<SOUTH>(shift<SOUTHWEST>(b));
		result |= shift<WEST>(shift<SOUTHWEST>(b));
		result |= shift<WEST>(shift<NORTHWEST>(b));
		result |= shift<NORTH>(shift<NORTHWEST>(b));

		return result;
	}

	Bitboard Bitboards::bishop_attacks(Bitboard b, Bitboard empty) {
		return
			ray<NORTHEAST>(b, empty) | ray<SOUTHEAST>(b, empty) |
			ray<SOUTHWEST>(b, empty) | ray<NORTHWEST>(b, empty);
	}

	Bitboard Bitboards::rook_attacks(Bitboard b, Bitboard empty) {
		return
			ray<NORTH>(b, empty) | ray<EAST>(b, empty) |
			ray<SOUTH>(b, empty) | ray<WEST>(b, empty);
	}

	Bitboard Bitboards::king_attacks(Bitboard b) {
		Bitboard result = EMPTY;

		result |= shift<NORTH>(b);
		result |= shift<NORTHEAST>(b);
		result |= shift<EAST>(b);
		result |= shift<SOUTHEAST>(b);
		result |= shift<SOUTH>(b);
		result |= shift<SOUTHWEST>(b);
		result |= shift<WEST>(b);
		result |= shift<NORTHWEST>(b);

		return result;
	}

}