#include "bitboard.hpp"

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace chess;

const static std::string startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w QKqk - 0 1";

// positions where each rook should be when they can still castle
const Bitmap rightStartingRooks[2] = { 1ULL, 1ULL << 56 };
const Bitmap leftStartingRooks[2] = { 1ULL << 7, 1ULL << 63 };

void chess::printmap(Bitmap bitmap)
{
	Bitmask position = 1ULL<<63;
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
}

int chess::getNextPosition(Bitmap& bitmap) {
	if (bitmap == 0) return -1;
	Bitmap positionMap = bitmap & ( ~bitmap + 1 );

	bitmap = bitmap ^ positionMap;

	int count = 0;

	if (positionMap & 0xffffffff00000000) count += 32;
	if (positionMap & 0xffff0000ffff0000) count += 16;
	if (positionMap & 0xff00ff00ff00ff00) count += 8;
	if (positionMap & 0xf0f0f0f0f0f0f0f0) count += 4;
	if (positionMap & 0xcccccccccccccccc) count += 2;
	if (positionMap & 0xaaaaaaaaaaaaaaaa) count += 1;

	return count;
}

Log::Log(BoardState boardInfo) : info(boardInfo) {}

void Log::addPiece(Piece piece, Bitmap mapPosition) {
	int intPosition = getSinglePosition(mapPosition);
	pieceInfo.push_back(PieceInfo{ piece, mapPosition, intPosition });
}

BoardState& chess::Board::getGameState() {
	BoardState out = { epMap, leftCastles[WHITE], rightCastles[WHITE], leftCastles[BLACK], rightCastles[BLACK], currentMove, halfMoves, colour };
	return out;
}



void Board::setPositions(Bitmap wp, Bitmap wn, Bitmap wb, Bitmap wr, Bitmap wq, Bitmap wk, Bitmap bp, Bitmap bn, Bitmap bb, Bitmap br, Bitmap bq, Bitmap bk){
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

	std::string layout   = data[0];
	std::string clr      = data[1];
	std::string castles  = data[2];
	std::string pawnMove = data[3];
	std::string halfMove = data[4];
	std::string fullMove = data[5];

	std::map<char, int> colourValues { {'P', 0}, {'N', 1}, {'B', 2}, {'R', 3}, {'Q', 4}, {'K', 5},
									  {'p', 6}, {'n', 7}, {'b', 8}, {'r', 9}, {'q', 10}, {'k', 11} };

	// getting positions of pieces
	Bitmap pos[12] { 0ULL };
	Bitmask position = 1ULL << 63;
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
	Bitmask position = 1ULL << 63;
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
	currentlyAltering.clear();
	Log newLog(getGameState());

	// search for moved piece
	Type startPieceType = 0;
	for (Bitmap posMap : positions[colour]) {
		if (posMap & move.start) {
			break;
		}
		startPieceType++;
	}

	if (startPieceType > KING) {
		throw std::invalid_argument("Cannot move from an empty square");
	}
	

	// move piece
	switch (move.flag) {
	case Flag::NONE:       movePieceDefault(move.start, move.end, startPieceType); break;
	case Flag::EN_PASSANT: movePieceEp(move.start, move.end); break;
	case Flag::PROMOTION:  movePiecePromotion(move.start, move.end, move.promotionPiece); break;
	case Flag::CASTLE:     movePieceCastle(move.start, move.end); break;
	}

	// search for taken piece
	Type endPieceType = 0;
	for (Bitmap posmap : positions[!colour]) {
		if (posmap & move.end) {
			break;
		}
		endPieceType++;
	}

	bool isCapture = endPieceType <= KING;
	// remove the taken piece
	if (isCapture) {
		takePiece(move.end, endPieceType);
	}

	// update halfMoves
	if (!isCapture && move.flag != Flag::EN_PASSANT) {
		halfMoves = 0;
	}

	// update epMap
	epMap = 0;
	if (startPieceType == PAWN) {
		updateEpMap(move.start, move.end);
	}

	// remove castle rights after a king move
	if (startPieceType == KING) {
		rightCastles[colour] = false;
		leftCastles[colour] = false;
	}

	for (std::pair<Piece, Bitmap> alter : currentlyAltering) {
		newLog.addPiece(alter.first, alter.second);
	}

	logs.push_back(newLog);

	currentlyAltering.clear();

	updateTeamPositions();
	incrementGameState();
}

void chess::Board::movePieceDefault(Bitmap start, Bitmap end, Type pieceType) {
	positions[colour][pieceType] ^= start | end;
	if (pieceType == ROOK) {
		removeSingleCastle(start, colour);
	}

	Piece alteringPiece(pieceType, colour);
	currentlyAltering.push_back({ alteringPiece, start });
	currentlyAltering.push_back({ alteringPiece, end });
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

	// pawn's original position
	currentlyAltering.push_back({ Piece(PAWN, colour), start });
	// promotion new position
	currentlyAltering.push_back({ Piece(promotionType, colour), end });
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

	if (pieceType == ROOK) removeSingleCastle(position, !colour);

	Piece alteringPiece(pieceType, !colour);
	currentlyAltering.push_back({ alteringPiece, position });
}

void chess::Board::updateEpMap(Bitmap start, Bitmap end) {
	if (colour == WHITE) {
		// end is not far enough away to have been doubled
		if ( (end >> 10) < start) return;
		epMap = end >> 8;
	}else { // BLACK
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
	if ((rookPosition & leftStartingRooks[colour]) && leftCastles[colour]) {
		leftCastles[colour] = false;
	}
	else if ((rookPosition & rightStartingRooks[colour]) && rightCastles[colour]) {
		rightCastles[colour] = false;
	}
}

void chess::Board::unmakeMove() {
	Log log = logs.back();
	logs.pop_back();
	for (PieceInfo info : log.pieceInfo) {
		positions[info.piece.colour][info.piece.type] ^= info.mapPosition;
	}
	BoardState info = log.info;
	setGameState(info.epMap, info.wlc, info.wrc, info.blc, info.brc, info.moveCount, info.halfMoveCount, info.colour);
	updateTeamPositions();
}

void chess::Board::print() {
	std::cout << toString() << std::endl;
}


