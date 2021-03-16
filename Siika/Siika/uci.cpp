#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>

#include <chrono>

#include "uci.h"
#include "debug.h"

namespace Chess {

	u64 UCI::nodes = 0;
	u32 UCI::self_depth = 0;

	std::atomic<bool> UCI::is_searching(false);
	std::thread UCI::search_thread;
	bool UCI::is_initialized = initialize();
	bool UCI::is_running = false;


	bool UCI::initialize() {
		return true;
	}

	void UCI::quit() {
		is_running = false;
		stop_searching();
	}

	void UCI::stop() {
		stop_searching();
	}

	void UCI::uci() {
		std::cout << "id author " << AUTHOR << "\n";
		std::cout << "id name " << ENGINE << " " << MAJOR_VERSION << "." << MINOR_VERSION << "\n";
		std::cout << "uciok\n";
	}

	void UCI::isready() {
		std::cout << "readyok\n";
	}

	void UCI::ucinewgame() {

	}

	void UCI::parse_position(std::istringstream& ss, PositionParameters& pp) {
		pp.from_startpos = true;
		pp.fen = "";
		pp.moves.clear();

		std::string token;
		ss >> token;
		if (token == "startpos") {
			pp.from_startpos = true;
			ss >> token;
		}
		else if (token == "fen") {
			std::stringstream fen;
			while (ss >> token) {
				if (token == "moves") {
					break;
				}
				fen << token << " ";
			}
			pp.from_startpos = false;
			pp.fen = fen.str();
		}
		if (token == "moves") {
			while (ss >> token) {
				pp.moves.push_back(token);
			}
		}
	}

	void UCI::parse_go(std::istringstream& ss, SearchParameters& sp) {
		if (!is_searching.load()) {
			std::string token;

			while (ss >> token) {
				if (token == "searchmoves") {

				}
				else if (token == "ponder") {

				}
				else if (token == "wtime") {
					ss >> sp.wtime_ms;
				}
				else if (token == "btime") {
					ss >> sp.btime_ms;
				}
				else if (token == "winc") {
					ss >> sp.winc_ms;
				}
				else if (token == "binc") {
					ss >> sp.binc_ms;
				}
				else if (token == "movestogo") {
					sp.is_sudden_death = false;
					ss >> sp.moves_to_go;
				}
				else if (token == "depth") {
					ss >> sp.max_depth;
				}
				else if (token == "nodes") {
					ss >> sp.max_nodes;
				}
				else if (token == "mate") {
					sp.search_for_mate = true;
					ss >> sp.search_for_mate_in_n;
				}
				else if (token == "movetime") {
					ss >> sp.max_search_time_ms;
				}
				else if (token == "infinite") {
					sp.max_depth = MAX_SEARCH_DEPTH;
					sp.wtime_ms = REALLY_BIG_NUMBER;
					sp.btime_ms = REALLY_BIG_NUMBER;
				}
			}
		}
	}

	void UCI::debug_perft(std::istringstream& ss, PositionParameters& pp) {
		Position pos;
		make_position(pp, pos);

		int depth = 0;
		ss >> depth;
		unsigned long long nodes = 0;
		auto start = std::chrono::high_resolution_clock::now();
		for (const auto move : pos.legal_moves()) {
			pos.do_move(move);
			auto move_nodes = Debug::perft(pos, depth - 1);
			std::cout << move_to_string(move) << ": " << move_nodes << "\n";
			nodes += move_nodes;
			pos.undo_move(move);
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "Leaf nodes: " << nodes << "\n";
		auto secs = std::chrono::duration<double>(end - start).count();
		auto nps = nodes / secs;
		std::string prefix = "";
		if (nps > 1e6) { prefix = "m"; nps /= 1e6; }
		else if (nps > 1e3) { prefix = "k"; nps /= 1e3; }
		std::cout << "Time elapsed: " << std::setprecision(2) << secs << " s (" << std::setprecision(3) << nps << " " << prefix << "nps)\n";
	}

	void UCI::debug_print(PositionParameters& pp) {
		Position pos;
		make_position(pp, pos);

		std::cout << pos;
	}

	void UCI::run() {
		is_running = true;
		PositionParameters p;
		while (is_running) {
			std::string input, token;
			std::getline(std::cin, input);
			std::istringstream iss(input);
			iss >> token;

			if (token == "quit") {
				quit();
			}
			else if (token == "stop") {
				stop();
			}
			else if (token == "uci") {
				uci();
			}
			else if (token == "isready") {
				isready();
			}
			else if (token == "go") {
				if (!is_searching.load()) {
					stop();
					SearchParameters sp;
					parse_go(iss, sp);
					if (!is_searching.load()) {
						search_thread = std::thread(search, std::ref(p), std::ref(sp));
					}
				}
			}
			else if (token == "position") {
				parse_position(iss, p);
			}
			// debug stuff
			else if (token == "perft") {
				debug_perft(iss, p);
			}
			else if (token == "d") {
				debug_print(p);
			}
		}
	}

	std::string UCI::square_to_string(int square) {
		if (square == NO_SQUARE) {
			return "-";
		}
		std::stringstream ss;
		ss << (char)('a' + square_file(square)) << (char)('1' + square_rank(square));
		return ss.str();
	}

	std::string UCI::move_to_string(Move move) {
		if (move == NULLMOVE) {
			return "0000";
		}
		std::stringstream ss;
		ss << square_to_string(move_from(move)) << square_to_string(move_to(move));

		if (is_promotion(move)) {
			switch (promotion_type(move)) {
			case QUEEN: ss << 'q'; break;
			case ROOK: ss << 'r'; break;
			case BISHOP: ss << 'b'; break;
			case KNIGHT: ss << 'n'; break;
			}
		}
		return ss.str();
	}

	Move UCI::parse_move(Position& pos, const std::string& str) {
		if (str.length() >= 4 || str != "0000") {
			int from = make_square(str[1] - '1', str[0] - 'a');
			int to = make_square(str[3] - '1', str[2] - 'a');

			if (square_ok(from) && square_ok(to)) {
				int flags = NORMAL_MOVE;
				if (str.length() == 5) {
					switch (str[4]) {
					case 'q': flags = PROMOTION_QUEEN; break;
					case 'r': flags = PROMOTION_ROOK; break;
					case 'b': flags = PROMOTION_BISHOP; break;
					case 'n': flags = PROMOTION_KNIGHT; break;
					}
				}
				Move move = make_move(from, to, flags);

				for (const auto candidate_move : pos.legal_moves()) {
					if (is_promotion(candidate_move)) {
						if (candidate_move == move) {
							return candidate_move;
						}
					}
					else if (move_from(candidate_move) == from && move_to(candidate_move) == to) {
						return candidate_move;
					}
				}
			}
		}
		return NULLMOVE;
	}

	void UCI::make_position(PositionParameters& pp, Position& pos) {
		if (pp.from_startpos) {
			pos.set_default();
		}
		else {
			pos.set(pp.fen);
		}

		for (const auto str : pp.moves) {
			auto move = parse_move(pos, str);
			if (move != NULLMOVE) {
				pos.do_move(move);
			}
		}
	}

	void UCI::stop_searching() {
		is_searching = false;
		if (search_thread.joinable()) {
			search_thread.join();
		}
	}

	

	Value UCI::negamax_ab(Position& pos, Value alpha, Value beta, int depth, int max_depth, Move& bestmove, bool is_root) {
		
		if (depth > self_depth) {
			self_depth = depth;
		}

		if (depth >= max_depth) {
			return quiescence_search(pos, alpha, beta, depth + 1);
		}
		nodes++;

		auto moves = pos.legal_moves();
		sort_moves(pos, moves);

		for (const auto move : moves) {
			pos.do_move(move);
			Value value = -negamax_ab(pos, -beta, -alpha, depth + 1, max_depth, bestmove, false);
			pos.undo_move(move);

			if (value >= beta) {
				return beta;
			}
			if (value > alpha) {
				alpha = value;
				if (is_root) {
					bestmove = move;
				}
			}
			if (!is_searching.load()) {
				return alpha;
			}
		}

		if (moves.empty()) {
			if (pos.is_in_check()) {
				return mate(depth);
			}
			else {
				return STALEMATE;
			}
		}
		

		return alpha;
	}

	Value UCI::quiescence_search(Position& pos, Value alpha, Value beta, int depth) {

		if (depth > self_depth) {
			self_depth = depth;
		}
		Value standing_pat = evaluate(pos);

		if (standing_pat >= beta) {
			return beta;
		}
		if (alpha < standing_pat) {
			alpha = standing_pat;
		}

		auto in_check = pos.is_in_check();
		auto moves = in_check ? pos.legal_moves() : pos.legal_moves(true);

		sort_moves(pos, moves);

		for (const auto move : moves) {
			pos.do_move(move);
			Value value = -quiescence_search(pos, -beta, -alpha, depth + 1);
			pos.undo_move(move);

			if (value >= beta) {
				return beta;
			}
			if (value > alpha) {
				alpha = value;
			}
		}

		if (moves.empty()) {
			if (in_check) {
				return mate(depth);
			}
		}
		else {
			nodes++;
		}

		return alpha;
	}

	void UCI::search(PositionParameters& pp, SearchParameters& sp) {
		Position pos;
		make_position(pp, pos);

		constexpr int moves_to_go = 28;
		auto player_time_usecs = pos.turn() == WHITE ? sp.wtime_ms * 1000 : sp.btime_ms * 1000;
		auto allocated_time_usecs = (player_time_usecs / moves_to_go);
		Move bestmove = NULLMOVE;
		is_searching = true;
		nodes = 0;


		
		std::cout << "(time allocated: " << (double)(allocated_time_usecs / 1000000.0) << " s)" << std::endl;
		Timer search;

		
		
		constexpr double ad_hoc_ratio = 12.0;
		for (int i =1; i <= sp.max_depth; i++) {
			self_depth = 0;

			u64 node_count = nodes;

			Timer depth_timer;
			Value val = negamax_ab(pos, VALUE_MIN, VALUE_MAX, 0, i, bestmove);
			auto depth_time = depth_timer.get_elapsed_microseconds();

			auto predicted_time = (Timer::Microseconds)(ad_hoc_ratio * depth_time);

			auto nodes_searched = nodes - node_count;
			auto nps = 1000000.0 * (double)(nodes_searched) / depth_time;

			std::cout << "info depth " << i << " ";
			std::cout << "seldepth " << self_depth << " ";
			std::cout << "nodes " << nodes_searched << " nps " << (u64)nps << " ";
			if (is_mate(val)) {
				int n = plies_till_mate(val);
				std::cout << "score mate " << ((n+1)/2);
			}
			else {
				std::cout << "score cp " << val;
			}

			std::cout << " pv " << move_to_string(bestmove);
			
			std::cout << std::endl;

			if (!is_searching.load() || search.get_elapsed_microseconds() > (allocated_time_usecs)) {
				break;
			}
			if ((search.get_elapsed_microseconds() + predicted_time) > (allocated_time_usecs + allocated_time_usecs * 0.3)) {
				std::cout << "(Predicted time " << (predicted_time / 1000000.0) << " exceeds allocated time " << (allocated_time_usecs / 1000000.0) << ")\n";
				break;
			}
		}
		
		std::cout << "bestmove " << move_to_string(bestmove) << std::endl;
		std::cout << "(time: " << (double)(search.get_elapsed_microseconds() / 1000000.0) << " s)" << std::endl;
		is_searching = false;
	}

}