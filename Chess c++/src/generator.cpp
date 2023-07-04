#include "generator.hpp"

#include "magics.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "pieces.hpp"

#include <iostream>
#include <cassert>
#include <bit>

namespace chess
{
	const Bitmap pawnEndRows[2] = { 0xffULL << 48, 0xffULL << 8 };
	const Bitmap pawnDoubleStepRows[2] = { 0xffULL << 16, 0xffULL << 40 };
	const int pawnStepSizes[2] = { 8, -8 };
	const Bitmap AFile = 0x8080808080808080;

	// defined at bottom
	void loadPawnPseudoPushMoves(Bitmap arr[2][64]);

	void loadPawnPseudoAttackMoves(Bitmap arr[2][64]);

	void loadKnightPseudoMoves(Bitmap arr[64]);

	void loadBishopPseudoMoves(MagicLookup arr[64]);

	void loadRookPseudoMoves(MagicLookup arr[64]);

	void loadKingPseudoMoves(Bitmap arr[64]);

	Bitmap pawnPushLookup[2][64];
	Bitmap pawnAttackLookup[2][64];
	Bitmap knightPseudoLookup[64];
	MagicLookup bishopPseudoLookup[64];
	MagicLookup rookPseudoLookup[64];
	Bitmap kingPseudoLookup[64];
	

	struct Startup {
		Startup() {
			loadPawnPseudoPushMoves(pawnPushLookup);
			loadPawnPseudoAttackMoves(pawnAttackLookup);
			loadKnightPseudoMoves(knightPseudoLookup);
			loadBishopPseudoMoves(bishopPseudoLookup);
			loadRookPseudoMoves(rookPseudoLookup);
			loadKingPseudoMoves(kingPseudoLookup);
		}
	};

	Startup startup;

	Generator::Generator() {
		board = nullptr;
		moves = nullptr;
		enemyEmptyMask = 0ULL;
		checkMask = 0ULL;
		attackMask = 0ULL;
		std::fill(pinMasks, pinMasks + 64, 0ULL);
	}
	Generator::Generator(Board& pboard) {
		board = &pboard;
		moves = nullptr;
		enemyEmptyMask = 0ULL;
		checkMask = 0ULL;
		attackMask = 0ULL;
		std::fill(pinMasks, pinMasks + 64, 0ULL);
	}

	bool Generator::isCheck() {
		return !(checkMask == ~0);
	}

	void Generator::getLegalMoves(MoveList* moveList) {
		assert(board);
		moves = moveList;
		
		if (!board->positions[board->colour][KING]) return;

		loadEnemyEmptyMask();
		loadCheckMask();
		loadAttackMask();
		loadPinMasks();

		addPawnMoves();
		addKnightMoves();
		addBishopMoves();
		addRookMoves();
		addQueenMoves();
		addKingMoves();
		addCastleMoves();
		addPromotionMoves();
		addEnPassantMoves();
	}

	void Generator::loadEnemyEmptyMask() {
		enemyEmptyMask = ~board->teamMaps[board->colour];
	}

	void Generator::loadCheckMask() {
		checkMask = ~0ULL;

		bool alreadyChecked = false;

		int kingPos = getSinglePosition(board->positions[board->colour][KING]);

		Bitmap* enemies = board->positions[1 - board->colour];

		// knights
		Bitmap knightMap = pseudoKnight(kingPos) & enemies[KNIGHT];
		int numKnights = std::popcount(knightMap);
		if (numKnights >= 2) {
			checkMask = 0;
			return;
		}
		else if (numKnights == 1) {
			checkMask &= knightMap;
			alreadyChecked = true;
		}
		
		// pawns
		Bitmap pawnMap = pawnAttackLookup[board->colour][kingPos] & enemies[PAWN];
		int numPawns = std::popcount(pawnMap);
		if ((numPawns + alreadyChecked) >= 2) {
			checkMask = 0;
			return;
		}
		else if (numPawns == 1) {
			checkMask &= pawnMap;
			alreadyChecked = true;
		}
		
		// rooks
		Bitmap rookMap = pseudoRook(kingPos) & (enemies[ROOK] | enemies[QUEEN]);
		int numRooks = std::popcount(rookMap);
		if ((numRooks + alreadyChecked) >= 2) {
			checkMask = 0;
			return;
		}
		else if (numRooks == 1) {
			int rookPos = getSinglePosition(rookMap);
			checkMask &= (pseudoRook(kingPos) & pseudoRook(rookPos)) | rookMap;
		}
		// bishops
		Bitmap bishopMap = pseudoBishop(kingPos) & (enemies[BISHOP] | enemies[QUEEN]);
		int numBishops = std::popcount(bishopMap);
		if ((numBishops + alreadyChecked) >= 2) {
			checkMask = 0;
			return;
		}
		else if (numBishops == 1) {
			int bishopPos = getSinglePosition(bishopMap);
			checkMask &= (pseudoBishop(kingPos) & pseudoBishop(bishopPos)) | bishopMap;
		}
	}

	void Generator::loadAttackMask() {
		Bitmap teamKingMap = board->positions[board->colour][KING];
		// ignore the king
		board->positions[board->colour][KING] = 0ULL;
		board->all ^= teamKingMap;

		Bitmap result = 0ULL;

		Bitmap pawnMap = board->positions[1 - board->colour][PAWN];
		while (pawnMap) {
			int position = getNextPosition(pawnMap);
			result |= pawnAttackLookup[1-board->colour][position];
		}

		Bitmap knightMap = board->positions[1 - board->colour][KNIGHT];
		while (knightMap) {
			int position = getNextPosition(knightMap);
			result |= pseudoKnight(position);
		}

		Bitmap bishopMap = board->positions[1 - board->colour][BISHOP];
		while (bishopMap) {
			int position = getNextPosition(bishopMap);
			result |= pseudoBishop(position);
		}

		Bitmap rookMap = board->positions[1 - board->colour][ROOK];
		while (rookMap) {
			int position = getNextPosition(rookMap);
			result |= pseudoRook(position);
		}

		Bitmap queenMap = board->positions[1 - board->colour][QUEEN];
		while (queenMap) {
			int position = getNextPosition(queenMap);
			result |= pseudoQueen(position);
		}

		Bitmap kingMap = board->positions[1 - board->colour][KING];
		
		int position = getSinglePosition(kingMap);
		result |= pseudoKing(position);

		board->all |= teamKingMap;
		board->positions[board->colour][KING] = teamKingMap;

		attackMask = result;
	}

	void Generator::loadPinMasks() {
		for (int position = 0; position < 64; position++) {
			pinMasks[position] = ~0ULL;
		}
		
		int kingPos = getSinglePosition(board->positions[board->colour][KING]);

		Bitmap rookMap = board->positions[1 - board->colour][ROOK];
		Bitmap bishopMap = board->positions[1 - board->colour][BISHOP];
		Bitmap queenMap = board->positions[1 - board->colour][QUEEN];

		Bitmap rookPseudo = rookPseudoLookup[kingPos][board->all];
		Bitmap teamRookPseudo = rookPseudo & board->teamMaps[board->colour];
		while (teamRookPseudo) {
			Bitmap posMap = teamRookPseudo & (~teamRookPseudo + 1);
			teamRookPseudo &= teamRookPseudo - 1;

			Bitmap newRookPseudo = rookPseudoLookup[kingPos][board->all ^ posMap];
			if (newRookPseudo & (rookMap | queenMap) & ~rookPseudo) {
				int pos = getSinglePosition(posMap);
				Bitmap newPositionPseudo = rookPseudoLookup[pos][board->all];
				pinMasks[pos] &= newPositionPseudo & newRookPseudo;
			}
		}

		Bitmap bishopPseudo = bishopPseudoLookup[kingPos][board->all];
		Bitmap teamBishopPseudo = bishopPseudo & board->teamMaps[board->colour];
		while (teamBishopPseudo) {
			Bitmap posMap = teamBishopPseudo & (~teamBishopPseudo + 1);
			teamBishopPseudo &= teamBishopPseudo - 1;

			Bitmap newBishopPseudo = bishopPseudoLookup[kingPos][board->all ^ posMap];
			if (newBishopPseudo & (bishopMap | queenMap) & ~bishopPseudo) {
				int pos = getSinglePosition(posMap);
				Bitmap newPositionPseudo = bishopPseudoLookup[pos][board->all];
				pinMasks[pos] &= newPositionPseudo & newBishopPseudo;
			}
		}
	}

	void Generator::addMoves(int position, Bitmap map, Flag flag, Type promotionPiece) {
		Bitmap startMap = bitset[position];
		while (map) {
			Bitmap endMap = map & (~map+1);
			Move move = makeMove(getSinglePosition(startMap), getSinglePosition(endMap), flag, promotionPiece);
			moves->add(move);
			map = map & (map - 1);
		}
	}

	void Generator::addPawnMoves() {
		Bitmap posMap = board->positions[board->colour][PAWN];
		Bitmap endRow = pawnEndRows[board->colour];
		posMap &= ~endRow;  // pawns on the end row  are dealt with in addPromotionMoves()
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmap pseudoMoves = pseudoPawn(position);
			Bitmap pinMask = pinMasks[position];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addKnightMoves() {
		Bitmap posMap = board->positions[board->colour][KNIGHT];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmap pseudoMoves = knightPseudoLookup[position];
			Bitmap pinMask = pinMasks[position];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addBishopMoves() {
		Bitmap posMap = board->positions[board->colour][BISHOP];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmap pseudoMoves = pseudoBishop(position);
			Bitmap pinMask = pinMasks[position];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addRookMoves() {
		Bitmap posMap = board->positions[board->colour][ROOK];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmap pseudoMoves = pseudoRook(position);
			Bitmap pinMask = pinMasks[position];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
			
		}
	}

	void Generator::addQueenMoves() {
		Bitmap posMap = board->positions[board->colour][QUEEN];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmap pseudoMoves = pseudoQueen(position);
			Bitmap pinMask = pinMasks[position];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}
	
	void Generator::addKingMoves() {
		int position = getSinglePosition(board->positions[board->colour][KING]);
		Bitmap pseudoMoves = pseudoKing(position);
		Bitmap pinMask = pinMasks[position];
		Bitmap legalMoves = pseudoMoves & ~attackMask & enemyEmptyMask;
		addMoves(position, legalMoves, Flag::NONE, PAWN);
	}

	void Generator::addCastleMoves() {
		
		Bitmap kingMap = board->positions[board->colour][KING];

		Bitmap otherPositions = board->all & ~kingMap;

		if (board->rightCastles[board->colour]) {
			Bitmap coveredPositions = kingMap | (kingMap >> 1) | (kingMap >> 2);
			if (!(coveredPositions & attackMask) && !(coveredPositions & otherPositions)) {
				addMoves(getSinglePosition(kingMap), kingMap >> 2, Flag::CASTLE, PAWN);
			}
		}
		if (board->leftCastles[board->colour]) {
			Bitmap coveredPositions = kingMap | (kingMap << 1) | (kingMap << 2) | (kingMap << 3);
			Bitmap noAttackPositions = kingMap | (kingMap << 1) | (kingMap << 2);
			if (!(noAttackPositions & attackMask) && !(coveredPositions & otherPositions)) {
				addMoves(getSinglePosition(kingMap), kingMap << 2, Flag::CASTLE, PAWN);
			}
		}
	}

	void Generator::addPromotionMoves() {
		constexpr Type promotionPieces[4] = { KNIGHT, BISHOP, ROOK, QUEEN };

		Bitmap endRow = pawnEndRows[board->colour];
		Bitmap startMap = board->positions[board->colour][PAWN] & endRow;

		while (startMap) {
			int pos = getNextPosition(startMap);
			Bitmap pseudoMoves = pseudoPawn(pos);
			Bitmap pinMask = pinMasks[pos];
			Bitmap legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			for (Type type : promotionPieces) {
				addMoves(pos, legalMoves, Flag::PROMOTION, type);
			}
		}
	}

	void Generator::addEnPassantMoves() {
		if (!board->epMap) return;
		int epPos = getSinglePosition(board->epMap);
		Bitmap pawnMap = pawnPushLookup[1 - board->colour][epPos];

		if (!(pawnMap & checkMask)) return;

		Bitmap startMap = pawnAttackLookup[1 - board->colour][epPos];
		startMap &= board->positions[board->colour][PAWN];

		int kingPos = getSinglePosition(board->positions[board->colour][KING]);
		Bitmap rookMap = board->positions[1 - board->colour][ROOK];
		Bitmap bishopMap = board->positions[1 - board->colour][BISHOP];
		Bitmap queenMap = board->positions[1 - board->colour][QUEEN];

		while (startMap) {
			int pos = getNextPosition(startMap);
			Bitmap alteredAll = board->all ^ board->epMap ^ bitset[pos] ^ pawnMap;
			Bitmap rookAttacks = rookPseudoLookup[kingPos][alteredAll] & (rookMap | queenMap);
			Bitmap bishopAttacks = bishopPseudoLookup[kingPos][alteredAll] & (bishopMap | queenMap);
			if (!(rookAttacks | bishopAttacks)) addMoves(pos, board->epMap, Flag::EN_PASSANT, PAWN);
		}

	}

	Bitmap Generator::pseudoPawn(int pos) {
		Bitmap nall = ~board->all;
		Bitmap forwardMap = pawnPushLookup[board->colour][pos];
		forwardMap &= nall;  // can't step on another piece

		if (forwardMap & pawnDoubleStepRows[board->colour]) {
			forwardMap |= pawnPushLookup[board->colour][pos + pawnStepSizes[board->colour]] & nall;
		}
		
		Bitmap attackMap = pawnAttackLookup[board->colour][pos] & board->all;

		return forwardMap | attackMap;
	}

	Bitmap Generator::pseudoKnight(int pos) { 
		return knightPseudoLookup[pos];
	}

	Bitmap Generator::pseudoBishop(int pos) { 
		return bishopPseudoLookup[pos][board->all];
	}
	Bitmap Generator::pseudoRook(int pos) { 
		return rookPseudoLookup[pos][board->all];
	}
	Bitmap Generator::pseudoQueen(int pos) { 
		return pseudoBishop(pos) | pseudoRook(pos);
	}
	Bitmap Generator::pseudoKing(int pos) { 
		return kingPseudoLookup[pos];
	}

	void loadPawnPseudoPushMoves(Bitmap arr[2][64]) {
		for (int pos = 0; pos < 64; pos++) {
			Bitmap posMap = bitset[pos];
			arr[WHITE][pos] = posMap << 8;
			arr[BLACK][pos] = posMap >> 8;
		}
	}

	void loadPawnPseudoAttackMoves(Bitmap arr[2][64]) {
		for (int pos = 0; pos < 64; pos++) {
			int col = 7 - pos % 8; // a=0 ... h=7
			Bitmap posMap = bitset[pos];
			Bitmap whiteMask = 0, blackMask = 0;
			if (col != 7) { // not far right
				whiteMask |= posMap << 7;
				blackMask |= posMap >> 9;
			}
			if (col != 0) { // not far right
				whiteMask |= posMap << 9;
				blackMask |= posMap >> 7;
			}
			arr[WHITE][pos] = whiteMask;
			arr[BLACK][pos] = blackMask;
		}
	}

	void loadKnightPseudoMoves(Bitmap arr[64]) {
		for (int pos = 0; pos < 64; pos++) {
			Bitmap neighbours = 0ULL;
			int row = 7 - pos / 8, col =7 - pos % 8;
			if (row < 7) { // not on bottom row
				if (col < 6) neighbours |= bitset[pos - 10];
				if (col > 1) neighbours |= bitset[pos - 6];
			}
			if (row < 6) { // not second bottom row
				if (col < 7) neighbours |= bitset[pos - 17];
				if (col > 0) neighbours |= bitset[pos - 15];
			}
			if (row > 0) { // not top row
				if (col < 6) neighbours |= bitset[pos + 6];
				if (col > 1) neighbours |= bitset[pos + 10];
			}
			if (row > 1) { // not second top row
				if (col < 7) neighbours |= bitset[pos + 15];
				if (col > 0) neighbours |= bitset[pos + 17];
			}
			arr[pos] = neighbours;
		}
	}

	void loadBishopPseudoMoves(MagicLookup arr[64]) {
		for (int pos = 0; pos < 64; pos++) {
			arr[pos] = MagicLookup(pos, true);
		}
	}

	void loadRookPseudoMoves(MagicLookup arr[64]) {
		for (int pos = 0; pos < 64; pos++) {
			arr[pos] = MagicLookup(pos, false);
		}
	}

	void loadKingPseudoMoves(Bitmap arr[64]) {
		for (int pos = 0; pos < 64; pos++) {
			int row = pos / 8, col = 7 - pos % 8;
			Bitmap neighbours = 0ULL;
			if (row > 0) {
				neighbours |= bitset[pos - 8];
				if (col > 0)  neighbours |= bitset[pos - 7];
				if (col < 7)  neighbours |= bitset[pos - 9];
			}
			if (row < 7) {
				neighbours |= bitset[pos + 8];
				if (col > 0)  neighbours |= bitset[pos + 9];
				if (col < 7)  neighbours |= bitset[pos + 7];
			}
			if (col > 0) neighbours |= bitset[pos + 1];
			if (col < 7) neighbours |= bitset[pos - 1];

			arr[pos] = neighbours;
		}
	}


	MoveList::MoveList() {
		count = 0u;
	}

	MoveList::MoveList(Generator& generator) {
		count = 0;
		generator.getLegalMoves(this);
	}

	void MoveList::add(Move move) {
		moves[count] = move;
		count++;
	}

	MoveList::iterator MoveList::begin() {
		return moves;
	}

	MoveList::iterator MoveList::end() {
		return &moves[count];
	}

	size_t MoveList::size() {
		return count;
	}

	void MoveList::clear() {
		count = 0;
	}
}
	
