#include <sstream>
#include <iostream>

#include "position.h"
#include "debug.h"

namespace Chess {

	// Zobrist
	Zobrist::Zobrist() {
		black_number = PRNG::get_64();

		for (int i = 0; i < piece_numbers.size(); i++) {
			piece_numbers[i] = PRNG::get_64();
		}

		for (int i = 0; i < castling_numbers.size(); i++) {
			castling_numbers[i] = PRNG::get_64();
		}

		for (int i = 0; i < ep_file_numbers.size(); i++) {
			ep_file_numbers[i] = PRNG::get_64();
		}
	}

	// Position
	const std::string Position::DEFAULT = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	Zobrist Position::zobrist_;

	void Position::reset() noexcept {
		turn_ = WHITE;
		fullmove_ = 1;
		ply_ = 0;
		undo_[0] = { NO_SQUARE, NO_CASTLINGS, NO_PIECE, 0, 0, 0, 0 };

		squares_.fill(NO_PIECE);
		bitboards_.fill(Bitboards::EMPTY);
	}

	// Default starting position
	Position::Position() noexcept {
		set_default();
	}

	Position::Position(const std::string& fen) noexcept {
		set(fen);
	}

	void Position::set_default() noexcept {
		turn_ = WHITE;
		fullmove_ = 1;
		ply_ = 0;

		squares_ = {
			4, 2, 3, 5, 6, 3, 2, 4,
			1, 1, 1, 1, 1, 1, 1, 1,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			16, 16, 16, 16, 16, 16, 16, 16,
			9, 9, 9, 9, 9, 9, 9, 9,
			12, 10, 11, 13, 14, 11, 10, 12
		};
		bitboards_ = {
			0xFFFF,
			0xFF00,
			0x42,
			0x24,
			0x81,
			0x8,
			0x10,
			0x0,
			0xffff000000000000,
			0xff000000000000,
			0x4200000000000000,
			0x2400000000000000,
			0x8100000000000000,
			0x800000000000000,
			0x1000000000000000,
			0x0,
			0x0
		};

		undo_[0] = { NO_SQUARE, ALL_CASTLINGS, NO_PIECE, 0, 0, 0, 0 };
		undo_[0].hash_ = calculate_hash();


		for (int i = 0; i < SQUARE_COUNT; i++) {
			int p = piece_on(i);
			material_[piece_color(p)] += piecetype_values[piece_type(p)];
		}
	}

	void Position::set(const std::string& fen) noexcept {
		reset();
		Undo undo{ NO_SQUARE, NO_CASTLINGS, NO_PIECE, 0, 0 };

		std::istringstream iss(fen);
		std::string section;

		iss >> section;
		int s = A8;
		for (const auto c : section) {
			if (c == '/') { s -= 16; }
			else if (std::isdigit(c)) { s += (c - '0'); }
			else {
				switch (c) {
				case 'P': put_piece(WHITE_PAWN, s); break;
				case 'N': put_piece(WHITE_KNIGHT, s); break;
				case 'B': put_piece(WHITE_BISHOP, s); break;
				case 'R': put_piece(WHITE_ROOK, s); break;
				case 'Q': put_piece(WHITE_QUEEN, s); break;
				case 'K': put_piece(WHITE_KING, s); break;
				case 'p': put_piece(BLACK_PAWN, s); break;
				case 'n': put_piece(BLACK_KNIGHT, s); break;
				case 'b': put_piece(BLACK_BISHOP, s); break;
				case 'r': put_piece(BLACK_ROOK, s); break;
				case 'q': put_piece(BLACK_QUEEN, s); break;
				case 'k': put_piece(BLACK_KING, s); break;
				}
				++s;
			}
		}

		iss >> section;
		turn_ = section == "w" ? WHITE : BLACK;

		iss >> section;
		for (const auto c : section) {
			if (c == 'K') undo.cr_ |= WHITE_KINGSIDE;
			else if (c == 'Q') undo.cr_ |= WHITE_QUEENSIDE;
			else if (c == 'k') undo.cr_ |= BLACK_KINGSIDE;
			else if (c == 'q') undo.cr_ |= BLACK_QUEENSIDE;
		}

		iss >> section;
		if (section != "-") {
			int f = section[0] - 'a';
			int r = section[1] - '1';
			undo.ep_ = make_square(r, f);
		}

		iss >> undo.halfmove_ >> fullmove_;

		undo_[0] = undo;
		undo_[0].hash_ = calculate_hash();
	}

	std::string Position::fen() const noexcept {
		std::stringstream ss;

		int empty = 0;
		for (int r = RANK_8; r >= RANK_1; r--) {
			for (int f = FILE_A; f < FILE_COUNT; f++) {
				int p = piece_on(make_square(r, f));

				if (p == NO_PIECE) {
					empty++;
				}
				else {
					if (empty) {
						ss << empty;
						empty = 0;
					}
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
					ss << c;
				}
			}
			if (empty) {
				ss << empty;
				empty = 0;
			}
			if (r > RANK_1) {
				ss << '/';
			}
		}

		ss << ' ';
		ss << (turn_ == WHITE ? 'w' : 'b');

		ss << ' ';
		if (castling_rights() == NO_CASTLINGS) ss << '-';
		if (castling_rights() & WHITE_KINGSIDE) ss << 'K';
		if (castling_rights() & WHITE_QUEENSIDE) ss << 'Q';
		if (castling_rights() & BLACK_KINGSIDE) ss << 'k';
		if (castling_rights() & BLACK_QUEENSIDE) ss << 'q';

		ss << ' ';
		if (en_passant_square() == NO_SQUARE) {
			ss << '-';
		}
		else {
			ss << (char)('a' + square_file(en_passant_square())) << (char)('1' + square_rank(en_passant_square()));
		}

		ss << ' ' << halfmove() << ' ' << fullmove();

		return ss.str();
	}

	bool Position::is_3x_repeat() const noexcept {
		for (int i = 0; i <= ply_; i++) {
			u64 h = undo_[i].hash_;
			int n = 1;
			for (int j = i + 1; j <= ply_; j++) {
				if (undo_[j].hash_ == h) {
					n++;
				}
			}
			if (n >= 3) {
				return true;
			}
		}
		return false;
	}

	void Position::update_attacks() noexcept {
		Bitboard attacks = Bitboards::EMPTY;
		
	}

	void Position::do_move(Move move) noexcept {
		int f = move_from(move);
		int t = move_to(move);

		Undo undo = { NO_SQUARE, castling_rights(), NO_PIECE, halfmove() + 1, hash() };

		if (en_passant_square() != NO_SQUARE) {
			undo.hash_ ^= zobrist_.ep_file_numbers[square_file(en_passant_square())];
		}

		// xor moved piece from_square
		undo.hash_ ^= piece_hash(f);

		switch (move_flags(move)) {
		case PAWN_DOUBLE_PUSH: {
			move_piece(f, t);
			undo.halfmove_ = 0;
			undo.ep_ = (f + t) >> 1;
			undo.hash_ ^= zobrist_.ep_file_numbers[square_file(undo.ep_)];
		} break;
		case KINGSIDE_CASTLE: {
			move_piece(f, t);
			undo.hash_ ^= piece_hash(f + 3);
			move_piece(f + 3, f + 1);
			undo.hash_ ^= piece_hash(f + 1);
		} break;
		case QUEENSIDE_CASTLE: {
			move_piece(f, t);
			undo.hash_ ^= piece_hash(f - 4);
			move_piece(f - 4, f - 1);
			undo.hash_ ^= piece_hash(f - 1);
		} break;
		case EN_PASSANT_CAPTURE: {
			int capsq = t - pawn_up(turn());
			undo.hash_ ^= piece_hash(capsq);
			undo.cap_ = piece_on(capsq);
			undo.halfmove_ = 0;
			take_piece(capsq);
			move_piece(f, t);
		} break;
		case PROMOTION_QUEEN:
		case PROMOTION_ROOK:
		case PROMOTION_BISHOP:
		case PROMOTION_KNIGHT: {
			undo.cap_ = piece_on(t);
			if (undo.cap_ != NO_PIECE) {
				undo.hash_ ^= piece_hash(t);
			}
			undo.halfmove_ = 0;
			take_piece(t);
			take_piece(f);
			put_piece(make_piece(promotion_type(move), turn()), t);
		} break;
		default: {
			if (piece_type(piece_on(f)) == PAWN || piece_on(t) != NO_PIECE) {
				undo.halfmove_ = 0;
			}
			undo.cap_ = piece_on(t);
			if (undo.cap_ != NO_PIECE) {
				undo.hash_ ^= piece_hash(t);
			}
			take_piece(t);
			move_piece(f, t);
		}
		}

		// xor moved piece to_square
		undo.hash_ ^= piece_hash(t);

		// xor castlings
		undo.hash_ ^= zobrist_.castling_numbers[castling_rights()];

		if (f == A1 || t == A1) {
			undo.cr_ &= ~(WHITE_QUEENSIDE);
		}
		if (f == H1 || t == H1) {
			undo.cr_ &= ~(WHITE_KINGSIDE);
		}
		if (f == E1) {
			undo.cr_ &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
		}

		if (f == A8 || t == A8) {
			undo.cr_ &= ~(BLACK_QUEENSIDE);
		}
		if (f == H8 || t == H8) {
			undo.cr_ &= ~(BLACK_KINGSIDE);
		}
		if (f == E8) {
			undo.cr_ &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
		}

		undo.hash_ ^= zobrist_.castling_numbers[undo.cr_];

		// xor color
		undo.hash_ ^= zobrist_.black_number;

		ply_++;
		undo_[ply_] = undo;

		if (turn() == BLACK) fullmove_++;
		turn_ = opponent();
	}

	void Position::undo_move(Move move) noexcept {
		int f = move_from(move);
		int t = move_to(move);

		if (turn() == WHITE) fullmove_--;
		turn_ = opponent();

		switch (move_flags(move)) {
		case PAWN_DOUBLE_PUSH: {
			move_piece(t, f);
		} break;
		case KINGSIDE_CASTLE: {
			move_piece(t, f);
			move_piece(f + 1, f + 3);
		} break;
		case QUEENSIDE_CASTLE: {
			move_piece(t, f);
			move_piece(f - 1, f - 4);
		} break;
		case EN_PASSANT_CAPTURE: {
			int capsq = t - pawn_up(turn());
			put_piece(captured_piece(), capsq);
			move_piece(t, f);
		} break;
		case PROMOTION_QUEEN:
		case PROMOTION_ROOK:
		case PROMOTION_BISHOP:
		case PROMOTION_KNIGHT: {
			take_piece(t);
			put_piece(captured_piece(), t);
			put_piece(make_piece(PAWN, turn()), f);
		} break;
		default: {
			move_piece(t, f);
			put_piece(captured_piece(), t);
		}
		}
		ply_--;
	}

	void Position::knight_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept {
		while (p) {
			int from = Bitboards::pop(p);
			//Bitboard attacks = Bitboards::knight_attacks(Bitboards::make(from)) & target;
			Bitboard attacks = Bitboards::knight_attacks(from) & target;
			while (attacks) {
				int to = Bitboards::pop(attacks);
				moves.push_back(make_move(from, to));
			}
		}
	}

	void Position::bishop_moves(Bitboard p, Bitboard target, Bitboard empty, std::vector<Move>& moves) const noexcept {
		while (p) {
			int from = Bitboards::pop(p);
			Bitboard attacks = Bitboards::bishop_attacks(Bitboards::make(from), empty) & target;
			while (attacks) {
				int to = Bitboards::pop(attacks);
				moves.push_back(make_move(from, to));
			}
		}
	}

	void Position::rook_moves(Bitboard p, Bitboard target, Bitboard empty, std::vector<Move>& moves) const noexcept {
		while (p) {
			int from = Bitboards::pop(p);
			Bitboard attacks = Bitboards::rook_attacks(Bitboards::make(from), empty) & target;
			while (attacks) {
				int to = Bitboards::pop(attacks);
				moves.push_back(make_move(from, to));
			}
		}
	}

	void Position::king_moves(Bitboard p, Bitboard target, std::vector<Move>& moves) const noexcept {
		while (p) {
			int from = Bitboards::pop(p);
			//Bitboard attacks = Bitboards::king_attacks(Bitboards::make(from)) & target;
			Bitboard attacks = Bitboards::king_attacks(from) & target;
			while (attacks) {
				int to = Bitboards::pop(attacks);
				moves.push_back(make_move(from, to));
			}
		}
	}

	void Position::castling_moves(std::vector<Move>& moves) const noexcept {
		int us = turn();
		int them = opponent();

		if (!is_square_attacked(king_square(us), us)) {
			if ((us == WHITE && castling_rights() & WHITE_KINGSIDE) || (us == BLACK && castling_rights() & BLACK_KINGSIDE)) {
				if (!is_square_attacked(king_square(us) + 1, us) && !is_square_attacked(king_square(us) + 2, us)) {
					if (piece_on(king_square(us) + 1) == NO_PIECE && piece_on(king_square(us) + 2) == NO_PIECE) {
						moves.push_back(make_move(king_square(us), king_square(us) + 2, KINGSIDE_CASTLE));
					}
				}
			}
			if ((us == WHITE && castling_rights() & WHITE_QUEENSIDE) || (us == BLACK && castling_rights() & BLACK_QUEENSIDE)) {
				if (!is_square_attacked(king_square(us) - 1, us) && !is_square_attacked(king_square(us) - 2, us)) {
					if (piece_on(king_square(us) - 1) == NO_PIECE &&
						piece_on(king_square(us) - 2) == NO_PIECE &&
						piece_on(king_square(us) - 3) == NO_PIECE) {
						moves.push_back(make_move(king_square(us), king_square(us) - 2, QUEENSIDE_CASTLE));
					}
				}
			}
		}
	}

	bool Position::is_legal(Move move) noexcept {
		do_move(move);
		bool is_legal = !is_square_attacked(king_square(opponent()), opponent());
		undo_move(move);
		return is_legal;
	}

	bool Position::is_square_attacked(int square, int defender) const noexcept {
		Bitboard b = Bitboards::make(square);
		int attacker = color_flip(defender);

		if (Bitboards::bishop_attacks(b, empty()) & (pieces(BISHOP, attacker) | pieces(QUEEN, attacker))) {
			return true;
		}
		if (Bitboards::rook_attacks(b, empty()) & (pieces(ROOK, attacker) | pieces(QUEEN, attacker))) {
			return true;
		}
		if (Bitboards::knight_attacks(b) & pieces(KNIGHT, attacker)) {
			return true;
		}
		if (Bitboards::king_attacks(b) & pieces(KING, attacker)) {
			return true;
		}

		if (defender == WHITE) {
			if (Bitboards::pawn_attacks<WHITE>(b) & pieces(PAWN, attacker)) {
				return true;
			}
		}
		else {
			if (Bitboards::pawn_attacks<BLACK>(b) & pieces(PAWN, attacker)) {
				return true;
			}
		}


		return false;
	}

	std::vector<Move> Position::legal_moves(bool only_captures) noexcept {

	}

	std::vector<Move> Position::legal_moves(bool only_captures) noexcept {
		std::vector<Move> pseudo_legal;
		pseudo_legal.reserve(32);


		// First caps
		Bitboard target = pieces(opponent());
		turn() == WHITE ? pawn_moves<WHITE>(pieces(PAWN, turn()), target, pseudo_legal) : pawn_moves<BLACK>(pieces(PAWN, turn()), target, pseudo_legal);
		knight_moves(pieces(KNIGHT, turn()), target, pseudo_legal);
		bishop_moves(pieces(BISHOP, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
		rook_moves(pieces(ROOK, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
		king_moves(pieces(KING, turn()), target, pseudo_legal);

		if (!only_captures) {
			target = empty();
			turn() == WHITE ? pawn_moves<WHITE>(pieces(PAWN, turn()), target, pseudo_legal) : pawn_moves<BLACK>(pieces(PAWN, turn()), target, pseudo_legal);
			knight_moves(pieces(KNIGHT, turn()), target, pseudo_legal);
			bishop_moves(pieces(BISHOP, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
			rook_moves(pieces(ROOK, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
			king_moves(pieces(KING, turn()), target, pseudo_legal);
			castling_moves(pseudo_legal);
		}
		/*Bitboard target = 0;
		if (only_captures) {
			target = pieces(opponent());
		}
		else {
			target = ~pieces(turn());
		}
		turn() == WHITE ? pawn_moves<WHITE>(pieces(PAWN, turn()), target, pseudo_legal) : pawn_moves<BLACK>(pieces(PAWN, turn()), target, pseudo_legal);
		knight_moves(pieces(KNIGHT, turn()), target, pseudo_legal);
		bishop_moves(pieces(BISHOP, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
		rook_moves(pieces(ROOK, turn()) | pieces(QUEEN, turn()), target, empty(), pseudo_legal);
		king_moves(pieces(KING, turn()), target, pseudo_legal);
		if (!only_captures) {
			castling_moves(pseudo_legal);
		}*/
		

		std::vector<Move> legal;
		legal.reserve(32);

		for (const auto move : pseudo_legal) {
			if (is_legal(move)) {
				legal.push_back(move);
			}
		}

		return legal;
	}

	u64 Position::calculate_hash() const {
		u64 hash = 0;
		for (int s = A1; s < SQUARE_COUNT; s++) {
			int p = piece_on(s);
			if (p != NO_PIECE) {
				hash ^= piece_hash(s);
			}
		}

		if (turn() == BLACK) {
			hash ^= zobrist_.black_number;
		}

		hash ^= zobrist_.castling_numbers[castling_rights()];

		if (en_passant_square() != NO_SQUARE) {
			hash ^= zobrist_.ep_file_numbers[square_file(en_passant_square())];
		}
		return hash;
	}

}