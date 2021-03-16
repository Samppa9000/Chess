#pragma once

#ifndef DEBUG_H
#define DEBUG_H

#include <ostream>

#include "bitboard.h"
#include "position.h"

namespace Chess {
	std::ostream& operator<<(std::ostream& os, const Position& pos);

	namespace Debug {
		std::string bitboard_to_string(Bitboard b);

		unsigned long long perft(Position& pos, int depth);
		void perft_suite(const std::string& file);
	}
}

#endif // DEBUG_H