#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iostream>

#include "debug.h"
#include <iomanip>

namespace Chess {
	std::ostream& operator<<(std::ostream& os, const Position& pos) {
		os << "   | a | b | c | d | e | f | g | h |\n";
		os << "---+---+---+---+---+---+---+---+---+---\n";
		for (int r = RANK_8; r >= RANK_1; r--) {
			os << " " << (r + 1) << " |";
			for (int f = FILE_A; f < FILE_COUNT; f++) {
				int p = pos.piece_on(make_square(r, f));
				char c;
				switch (p) {
				case WHITE_PAWN: c = 'P'; break;
				case WHITE_KNIGHT: c = 'N'; break;
				case WHITE_BISHOP: c = 'B'; break;
				case WHITE_ROOK: c = 'R'; break;
				case WHITE_QUEEN: c = 'Q'; break;
				case WHITE_KING: c = 'K'; break;
				case BLACK_PAWN: c = 'p'; break;
				case BLACK_KNIGHT: c = 'n'; break;
				case BLACK_BISHOP: c = 'b'; break;
				case BLACK_ROOK: c = 'r'; break;
				case BLACK_QUEEN: c = 'q'; break;
				case BLACK_KING: c = 'k'; break;
				default: c = ' ';
				}
				os << " " << c << " |";
			}
			os << " " << (r + 1) << "\n";
			os << "---+---+---+---+---+---+---+---+---+---\n";
		}
		os << "   | a | b | c | d | e | f | g | h |\n\n";
		os << "FEN: " << pos.fen() << "\n";
		os << "Hash: " << std::hex << pos.hash() << "\n";
		os << "3x rep: " << std::boolalpha << pos.is_3x_repeat() << "\n";

		return os;
	}

	namespace Debug {
		std::string bitboard_to_string(Bitboard b) {
			std::stringstream os;

			os << "   | a | b | c | d | e | f | g | h |\n";
			os << "---+---+---+---+---+---+---+---+---+---\n";
			for (int r = RANK_8; r >= RANK_1; r--) {
				os << " " << (r + 1) << " |";
				for (int f = FILE_A; f < FILE_COUNT; f++) {
					int s = make_square(r, f);
					char c = (Bitboards::make(s) & b) ? 'X' : ' ';

					os << " " << c << " |";
				}
				os << " " << (r + 1) << "\n";
				os << "---+---+---+---+---+---+---+---+---+---\n";
			}
			os << "   | a | b | c | d | e | f | g | h |\n";

			return os.str();
		}

		unsigned long long perft(Position& pos, int depth) {
			if (depth <= 0) {
				return 1;
			}

			unsigned long long nodes = 0;
			for (const auto move : pos.legal_moves()) {
				pos.do_move(move);
				unsigned long long move_nodes = perft(pos, depth - 1);

				nodes += move_nodes;
				pos.undo_move(move);
			}

			return nodes;
		}
		void perft_suite(const std::string& file) {
			std::ifstream ifs(file.c_str());

			if (!ifs.good()) {
				return;
			}

			std::vector<std::string> incorrect;
			unsigned long long total_nodes = 0;

			std::stringstream buffer;
			buffer << ifs.rdbuf();

			std::vector<std::string> lines;
			std::string line;
			int correct = 0, total = 0;

			while (std::getline(buffer, line)) {
				lines.push_back(line);
			}

			Position pos;

			auto start = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < lines.size(); i++) {
				std::string fen, token;

				std::istringstream iss(lines[i]);

				std::getline(iss, fen, ';');
				pos.set(fen);
				std::cout << i << "/" << lines.size() << " ";
				std::cout << "FEN: " << fen << "\n";
				while (std::getline(iss, token, ';')) {
					token.erase(std::remove(token.begin(), token.end(), 'D'), token.end());
					std::istringstream ss(token);

					int depth = 0;
					unsigned long long nodes = 0;
					ss >> depth >> nodes;

					auto result = perft(pos, depth);
					std::cout << "perft(" << depth << "): " << result << " (" << nodes << ")\n";
					total_nodes += result;
					if (result != nodes) {
						incorrect.push_back(fen);
					}
					else {
						correct++;
					}
					total++;
				}
			}

			auto end = std::chrono::high_resolution_clock::now();


			std::cout << "Leaf nodes: " << total_nodes << "\n";
			auto secs = std::chrono::duration<double>(end - start).count();
			auto nps = total_nodes / secs;
			std::string prefix = "";
			if (nps > 1E6) { prefix = "M"; nps /= 1E6; }
			else if (nps > 1E3) { prefix = "k"; nps /= 1E3; }
			std::cout << "Time elapsed: " << std::setprecision(2) << secs << " s (" << std::setprecision(3) << nps << " " << prefix << "nps)\n";

			std::cout << correct << " out of " << total << " correct.\n";

			for (const auto c : incorrect) {
				std::cout << "Incorrect: " << c << "\n";
			}


		}
	}
}