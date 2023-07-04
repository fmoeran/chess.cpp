#include "bitboard.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

//#pragma intrinsic(_BitScanForward)

namespace chess
{

	const static std::string startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w QKqk - 0 1";

	// positions where each rook should be when they can still castle
	const Bitmap rightStartingRooks[2] = { 1ULL, 1ULL << 56 };
	const Bitmap leftStartingRooks[2] = { 1ULL << 7, 1ULL << 63 };

	void chess::printmap(Bitmap bitmap)
	{
		Bitmap position = 1ULL << 63;
		for (int row = 0; row < 8; row++) {
			for (int col = 0; col < 8; col++) {
				if (position & bitmap) std::cout << '1';
				else                   std::cout << '.';
				position >>= 1;
			}
			std::cout << '\n';
		}
		std::cout << std::endl;
	}


	int chess::getSinglePosition(Bitmap bitmap)
	{

		unsigned long out;
		if (!_BitScanForward64(&out, bitmap)) return 0;
		return out;
		/*
		equal to:

		if (bitmap == 0) return -1;
		bitmap &= ~bitmap + 1;

		int count = 0;

		if (bitmap & 0xffffffff00000000) count += 32;
		if (bitmap & 0xffff0000ffff0000) count += 16;
		if (bitmap & 0xff00ff00ff00ff00) count += 8;
		if (bitmap & 0xf0f0f0f0f0f0f0f0) count += 4;
		if (bitmap & 0xcccccccccccccccc) count += 2;
		if (bitmap & 0xaaaaaaaaaaaaaaaa) count += 1;

		return count;
		*/

	}

	int chess::getNextPosition(Bitmap& bitmap) {
		if (bitmap == 0) return -1;
		Bitmap positionMap = bitmap & (~bitmap + 1);

		bitmap = bitmap ^ positionMap;

		return getSinglePosition(positionMap);
	}

	BoardState createState(int moveCount, int halfMoveCount, bool wlc, bool wrc, bool blc, bool brc, Bitmap epMap, bool capture, Type captureType) {
		return (moveCount) | (halfMoveCount << 14) |
			((int)wlc << 20) | ((int)wrc << 21) | ((int)blc << 22) | ((int)brc << 23) |
			((bool)epMap << 24) | ((getSinglePosition(epMap) % 8) << 25) | (capture << 28) | ((int)captureType << 29);
	}


	int getMoveCount(BoardState state) {
		return state & 0b1111111111111; // 13 bits
	}

	int getHalfMoveCount(BoardState state) {
		return (state >> 14) & 0b111111;
	}

	bool wlc(BoardState state) {
		return (state >> 20) & 1;
	}

	bool wrc(BoardState state) {
		return (state >> 21) & 1;
	}

	bool blc(BoardState state) {
		return (state >> 22) & 1;
	}

	bool brc(BoardState state) {
		return (state >> 23) & 1;
	}

	Bitmap getEpMap(BoardState state, Colour colour) {
		bool ep = (state >> 24) & 1;
		if (!ep) return 0ULL;
		int row = (state >> 25) & 0b111;
		if (colour == WHITE) return bitset[row + 40];
		else return bitset[row + 16];
	}

	bool isCapture(BoardState state) {
		return (state >> 28) & 1;
	}

	Type captureType(BoardState state) {
		return (state >> 29) & 0b111;
	}

	void Board::setPositions(Bitmap wp, Bitmap wn, Bitmap wb, Bitmap wr, Bitmap wq, Bitmap wk, Bitmap bp, Bitmap bn, Bitmap bb, Bitmap br, Bitmap bq, Bitmap bk) {
		Bitmap newPositions[2][6] = { {wp, wn, wb, wr, wq, wk}, {bp, bn, bb, br, bq, bk} };
		std::copy(newPositions[0], newPositions[0] + 6, positions[0]);
		std::copy(newPositions[1], newPositions[1] + 6, positions[1]);

		updateTeamPositions();
	}

	void Board::setGameState(Bitmap ep, bool wlc, bool wrc, bool blc, bool brc, int moveCount, int hm, Colour clr) {
		epMap = ep;
		rightCastles = { wrc, brc };
		leftCastles = { wlc, blc };
		currentMove = moveCount;
		colour = clr;
		halfMoves = hm;
	}

	Board::Board(Bitmap wp, Bitmap wn, Bitmap wb, Bitmap wr, Bitmap wq, Bitmap wk, Bitmap bp, Bitmap bn, Bitmap bb, Bitmap br, Bitmap bq, Bitmap bk, Bitmap ep, bool wlc, bool wrc, bool blc, bool brc, int move_count, int hm, Colour colour)
	{
		setPositions(wp, wn, wb, wr, wq, wk, bp, bn, bb, br, bq, bk);
		setGameState(ep, wlc, wrc, blc, brc, move_count, hm, colour);
	}
	Board::Board() {
		*this = Board::fromFen(startingFen);
	}

	Board Board::fromFen(std::string fen)
	{
		std::vector<std::string> data = { "" };
		for (char c : fen) {
			if (c == ' ') data.push_back("");
			else data.back() += c;
		}
		while (data.size() < 6) data.push_back("-");

		std::string layout = data[0];
		std::string clr = data[1];
		std::string castles = data[2];
		std::string pawnMove = data[3];
		std::string halfMove = data[4];
		std::string fullMove = data[5];

		std::map<char, int> colourValues{ {'P', 0}, {'N', 1}, {'B', 2}, {'R', 3}, {'Q', 4}, {'K', 5},
										  {'p', 6}, {'n', 7}, {'b', 8}, {'r', 9}, {'q', 10}, {'k', 11} };

		// getting positions of pieces
		Bitmap pos[12]{ 0ULL };
		Bitmap position = 1ULL << 63;
		for (char c : layout) {
			if (c == '/') continue;
			else if ('0' < c && c < '9') position >>= ((int)c - (int)'0');
			else {
				int pieceVal = colourValues[c];
				pos[pieceVal] |= position;
				position >>= 1;
			}
		}

		// colour
		Colour initColour;
		if (clr == "w") initColour = WHITE;
		else initColour = BLACK;
		// castling
		bool wlc = std::find(castles.begin(), castles.end(), 'Q') != castles.end();
		bool wrc = std::find(castles.begin(), castles.end(), 'K') != castles.end();
		bool blc = std::find(castles.begin(), castles.end(), 'q') != castles.end();
		bool brc = std::find(castles.begin(), castles.end(), 'k') != castles.end();
		// en passant bitmap
		Bitmap ep = 0;
		if (pawnMove != "-") {
			int column = std::distance(columnLetters, std::find(columnLetters, columnLetters + 8, pawnMove[0]));
			int row = 8 * (pawnMove[1] - '0' - 1);
			int lastMoveEnd = (8 - column) + (8 * row) - 1;
			ep = 1 < lastMoveEnd;
			if (initColour == WHITE) ep >>= 8;
			else ep <<= 8;
		}

		// half and full move counts
		int hm = 0;
		if (halfMove != "-") hm = std::stoi(halfMove);
		int moves = initColour;
		if (fullMove != "-") moves = std::stoi(fullMove);

		return Board(pos[0], pos[1], pos[2], pos[3], pos[4], pos[5], pos[6], pos[7], pos[8], pos[9], pos[10], pos[11], ep,
			wlc, wrc, blc, brc, moves, hm, initColour);
	}

	std::string chess::Board::toString()
	{
		Bitmap position = 1ULL << 63;
		std::string out = "";
		Colour clr;
		bool altered;
		for (int intPosition = 63; intPosition >= 0; intPosition--) {
			altered = false;
			clr = WHITE;

			if (teamMaps[BLACK] & position) clr = BLACK;

			for (int piece = 0; piece <= 6; piece++) {
				if (position & positions[clr][piece]) {
					out += getChar(piece, clr);
					altered = true;
					break;
				}
			}
			if (!altered) out += ".";
			position >>= 1;
			if (intPosition % 8 == 0) out += "\n";
		}
		return out;
	}

	void chess::Board::makeMove(Move move) {
		Bitmap start = bitset[getStart(move)];
		Bitmap end = bitset[getEnd(move)];
		Flag flag = getFlag(move);

		if (!(bool)(start & teamMaps[colour])) {
			throw std::invalid_argument("Cannot move from an empty square");
		}
	
		bool capture = (end & teamMaps[1 - colour]);
		Type endPieceType = 0;
		if (capture) {
			// search for taken piece
			for (Bitmap posmap : positions[!colour]) {
				if (posmap & end) {
					break;
				}
				endPieceType++;
			}
		}

		// update past states stack
		pastStates.push(createState(currentMove, halfMoves,
			leftCastles[WHITE], rightCastles[WHITE], leftCastles[BLACK], rightCastles[BLACK],
			epMap, capture, endPieceType));

		// this is done after adding to the state as taking the piece could affect castling rules
		if (capture) {
			takePiece(end, endPieceType);
		}

		// search for moved piece
		Type startPieceType = PAWN;
		for (Bitmap posMap : positions[colour]) {
			if (posMap & start) {
				break;
			}
			startPieceType++;
		}

		// move piece
		switch (flag) {
		case Flag::NONE:       movePieceDefault(start, end, startPieceType); break;
		case Flag::EN_PASSANT: movePieceEp(start, end); break;
		case Flag::PROMOTION:  movePiecePromotion(start, end, getPromotion(move)); break;
		case Flag::CASTLE:     movePieceCastle(start, end); break;
		}

		// update halfMoves
		if (!capture && flag != Flag::EN_PASSANT) {
			halfMoves = 0;
		}

		// update epMap
		epMap = 0;
		if (startPieceType == PAWN) {
			updateEpMap(start, end);
		}

		// remove castle rights after a king move
		if (startPieceType == KING) {
			rightCastles[colour] = false;
			leftCastles[colour] = false;
		}
		updateTeamPositions();
		incrementGameState();
	}

	void chess::Board::movePieceDefault(Bitmap start, Bitmap end, Type pieceType) {
		positions[colour][pieceType] ^= start | end;
		if (pieceType == ROOK) {
			removeSingleCastle(start, colour);
		}
	}

	void chess::Board::movePieceEp(Bitmap start, Bitmap end) {
		movePieceDefault(start, end, PAWN);
		// position of the piece taken
		Bitmap epPos;
		if (colour == WHITE) epPos = end >> 8;
		else                 epPos = end << 8;

		takePiece(epPos, PAWN);
	}

	void chess::Board::movePiecePromotion(Bitmap start, Bitmap end, Type promotionType) {
		positions[colour][PAWN] ^= start;
		positions[colour][promotionType] |= end;
	}

	void chess::Board::movePieceCastle(Bitmap start, Bitmap end) {
		movePieceDefault(start, end, KING);

		Bitmap rstart, rend;
		if (colour == WHITE) {
			if (end < start) { // castle right
				rstart = 1;
				rend = end << 1;
			}
			else {            // castle left
				rstart = 1 << 7;
				rend = end >> 1;
			}
		}
		else { // BLACK
			if (end < start) { // castle right
				rstart = 1ULL << 56;
				rend = end << 1;
			}
			else {            // castle left
				rstart = 1ULL << 63;
				rend = end >> 1;
			}
		}
		movePieceDefault(rstart, rend, ROOK);

	}

	void chess::Board::takePiece(Bitmap position, Type pieceType) {
		positions[!colour][pieceType] ^= position;

		if (pieceType == ROOK) removeSingleCastle(position, 1 - colour);
	}

	void chess::Board::updateEpMap(Bitmap start, Bitmap end) {
		if (colour == WHITE) {
			// end is not far enough away to have been doubled
			if ((end >> 10) < start) return;
			epMap = end >> 8;
		}
		else { // BLACK
		   // end is not far enough away to have been doubled
			if ((end << 10) > start) return;
			epMap = end << 8;
		}
	}

	void chess::Board::incrementGameState() {
		currentMove++;
		halfMoves++;
		colour = !colour;
	}

	void chess::Board::updateTeamPositions() {
		teamMaps[WHITE] = 0;
		for (Bitmap posmap : positions[WHITE]) {
			teamMaps[WHITE] |= posmap;
		}
		teamMaps[BLACK] = 0;
		for (Bitmap posmap : positions[BLACK]) {
			teamMaps[BLACK] |= posmap;
		}
		all = teamMaps[0] | teamMaps[1];
	}

	void chess::Board::removeSingleCastle(Bitmap rookPosition, Colour clr) {
		if ((rookPosition & leftStartingRooks[clr]) && leftCastles[clr]) {
			leftCastles[clr] = false;
		}
		else if ((rookPosition & rightStartingRooks[clr]) && rightCastles[clr]) {
			rightCastles[clr] = false;
		}
	}

	void chess::Board::unmakeMove(Move move) {
		BoardState state = pastStates.top();
		pastStates.pop();

		setGameState(getEpMap(state, !colour), wlc(state), wrc(state), blc(state), brc(state), getMoveCount(state), getHalfMoveCount(state), !colour);

		Bitmap start = bitset[getStart(move)];
		Bitmap end = bitset[getEnd(move)];
		Flag flag = getFlag(move);

		
		if (flag == Flag::NONE) {
			Type movePiece = 0;
			for (Bitmap posMap : positions[colour]) {
				if (posMap & end) {
					break;
				}
				movePiece++;
			}
			movePieceDefault(end, start, movePiece);
			if (isCapture(state)) {
				Type taken = captureType(state);
				positions[!colour][taken] ^= end;

			}
		}
		else if (flag == Flag::EN_PASSANT) {
			movePieceDefault(end, start, PAWN);
			Bitmap takenMap;
			if (colour == WHITE) takenMap = epMap >> 8;
			else                 takenMap = epMap << 8;
			positions[!colour][PAWN] ^= takenMap;

		}
		else if (flag == Flag::CASTLE) {
			movePieceDefault(end, start, KING);
			if (start > end) movePieceDefault(end << 1, end >> 1, ROOK);
			else             movePieceDefault(end >> 1, end << 2, ROOK);
		}
		else if (flag == Flag::PROMOTION) {
			Type promoPiece = getPromotion(move);
			positions[colour][promoPiece] ^= end;
			positions[colour][PAWN] ^= start;
			if (isCapture(state)) {
				Type taken = captureType(state);
				positions[!colour][taken] ^= end;
			}
		}
		updateTeamPositions();
	}

	void chess::Board::print() {
		std::cout << toString() << std::endl;
	}
}