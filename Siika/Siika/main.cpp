#include <iostream>

#include "position.h"
#include "bitboard.h"
#include "uci.h"
#include "debug.h"
#include "eval.h"
#include "util.h"

using namespace Chess;

int main() {

	UCI::run();

	/*Position pos("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");

	auto moves = pos.legal_moves();

	for (const auto move : moves) {
		std::cout << UCI::move_to_string(move) << "\n";
	}
	std::cout << "\n";
	sort_moves(pos, moves);
	for (const auto move : moves) {
		std::cout << UCI::move_to_string(move) << "\n";
	}*/

	return 0;
}