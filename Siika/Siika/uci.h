#pragma once

#ifndef UCI_H
#define UCI_H

#include <string>
#include <thread>
#include <atomic>
#include <map>
#include <functional>
#include <vector>

#include "chess.h"
#include "position.h"
#include "eval.h"

namespace Chess {

	constexpr u32 MAX_SEARCH_DEPTH = MAX_PLYS - 1;
	constexpr u64 REALLY_BIG_NUMBER = UINT64_MAX;

	class UCI {
	public:
		static void run();



		UCI() = delete;
	private:
		struct PositionParameters {
			bool from_startpos = true;
			std::string fen;
			std::vector<std::string> moves;
		};

		struct SearchParameters {
			u64 wtime_ms = 0, btime_ms = 0;
			u64 winc_ms = 0, binc_ms = 0;
			u32 max_depth = MAX_SEARCH_DEPTH;
			bool ponder = false;
			u64 max_nodes = REALLY_BIG_NUMBER;
			u64 max_search_time_ms = REALLY_BIG_NUMBER;
			bool search_for_mate = false;
			u32 search_for_mate_in_n = 0;
			bool is_sudden_death = true;
			u32 moves_to_go = 0;
			std::vector<Move> searchmoves;
		};

		static u64 nodes;
		static u32 self_depth;
		static std::atomic<bool> is_searching;
		static std::thread search_thread;
		static bool is_initialized;
		static bool is_running;
	public:
	//private:
		static bool initialize();

		static std::string square_to_string(int square);
		static std::string move_to_string(Move move);
		static Move parse_move(Position& pos, const std::string& str);
		static void make_position(PositionParameters& pp, Position& pos);

		static void stop_searching();
		static void search(PositionParameters& pp, SearchParameters& sp);
		static Value negamax_ab(Position& pos, Value alpha, Value beta, int depth, int max_depth, Move& bestmove, bool is_root = true);
		static Value quiescence_search(Position& pos, Value alpha, Value beta, int depth);


		static void quit();
		static void stop();
		static void uci();
		static void isready();
		static void ucinewgame();
		static void parse_position(std::istringstream& ss, PositionParameters& pp);
		static void parse_go(std::istringstream& ss, SearchParameters& sp);

		static void debug_perft(std::istringstream& ss, PositionParameters& pp);
		static void debug_print(PositionParameters& pp);

	};

}

#endif // UCI_H