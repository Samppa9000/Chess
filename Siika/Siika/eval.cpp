#include "eval.h"
#include <algorithm>


namespace Chess {
	
	

	void sort_moves(Position& pos, std::vector<Move>& moves) noexcept {
		// sort moves based on MVV-LVA 
		std::sort(moves.begin(), moves.end(), [&](Move lhs, Move rhs) {
			int lhs_val = 0, rhs_val = 0;
			
			auto lhs_captured = pos.piece_on(move_to(lhs));
			auto rhs_captured = pos.piece_on(move_to(rhs));

			auto lhs_attacker = pos.piece_on(move_from(lhs));
			auto rhs_attacker = pos.piece_on(move_from(rhs));

			if (lhs_captured != NO_PIECE) {
				lhs_val = piecetype_values[lhs_captured] - piecetype_values[lhs_attacker] + 900;
			}
			if (rhs_captured != NO_PIECE) {
				rhs_val = piecetype_values[rhs_captured] - piecetype_values[rhs_attacker] + 900;
			}
			
			

			return lhs_val > rhs_val;
		});


	}

}