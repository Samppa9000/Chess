

#pragma once

#ifndef EVAL_H
#define EVAL_H

#include "chess.h"
#include "position.h"

namespace Chess {

	
	constexpr Value MATE = -1000000;
	constexpr i32 MAX_PLIES_TILL_MATE = 256;
	constexpr Value VALUE_MAX = -MATE + MAX_PLIES_TILL_MATE;
	constexpr Value VALUE_MIN = MATE - MAX_PLIES_TILL_MATE;
	constexpr i32 STALEMATE = 0;
	constexpr i32 DRAW = 0;

	inline Value evaluate(const Position& pos) {
		if (pos.is_3x_repeat()) {
			return DRAW;
		}

		return pos.material_diff();
	}

	void sort_moves(Position& pos, std::vector<Move>& moves) noexcept;

	constexpr Value mate(int plies_till_mate) {
		return MATE - (MAX_PLIES_TILL_MATE - plies_till_mate);
	}

	constexpr bool is_mate(Value value) {
		return value <= MATE || value >= -MATE;
	}

	constexpr int plies_till_mate(Value value) {

		if (value > 0) {
			return VALUE_MAX - value;
		}
		else {
			return VALUE_MIN - value;
		}
	}
}

#endif // EVAL_H