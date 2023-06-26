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
	const Bitmask pawnEndRows[2] = { 0xffULL << 48, 0xffULL << 8 };
	const Bitmask pawnDoubleStepRows[2] = { 0xffULL << 16, 0xffULL << 40 };
	const int pawnStepSizes[2] = { 8, -8 };

	// defined at bottom
	void loadPawnPseudoPushMoves(Bitmask arr[2][64]);

	void loadPawnPseudoAttackMoves(Bitmask arr[2][64]);

	void loadKnightPseudoMoves(Bitmask arr[64]);

	void loadBishopPseudoMoves(MagicLookup arr[64]);

	void loadRookPseudoMoves(MagicLookup arr[64]);

	void loadKingPseudoMoves(Bitmask arr[64]);

	Bitmask pawnPushLookup[2][64];
	Bitmask pawnAttackLookup[2][64];
	Bitmask knightPseudoLookup[64];
	MagicLookup bishopPseudoLookup[64];
	MagicLookup rookPseudoLookup[64];
	Bitmask kingPseudoLookup[64];
	

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
		enemyEmptyMask = 0ULL;
		checkMask = 0ULL;
		attackMask = 0ULL;
		std::fill(pinMasks, pinMasks + 64, 0ULL);
	}
	Generator::Generator(Board& pboard) {
		board = &pboard;
		enemyEmptyMask = 0ULL;
		checkMask = 0ULL;
		attackMask = 0ULL;
		std::fill(pinMasks, pinMasks + 64, 0ULL);
	}

	std::vector<Move> Generator::getLegalMoves() {
		assert(board);

		moves.clear();
		
		if (!board->positions[board->colour][KING]) return moves;

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
		
		//addCastleMoves();
		//addPromotionMoves();
		//addEnPassantMoves();

		return moves;
	}

	void Generator::loadEnemyEmptyMask() {
		enemyEmptyMask = ~board->teamMaps[board->colour];
	}

	void Generator::loadCheckMask() {
		checkMask = ~0ULL;
	}

	void Generator::loadAttackMask() {}

	void Generator::loadPinMasks() {
		for (int position = 0; position < 64; position++) {
			pinMasks[position] = ~0ULL;
		}
	}


	void Generator::addMoves(int position, Bitmap map, Flag flag, Type promotionPiece) {
		moves.resize(moves.size() + std::popcount(map));
		Bitmap startMap = bitset[position];
		while (map) {
			
			Bitmap endMap = map & (~map+1);
			moves.push_back(Move(startMap, endMap, flag, promotionPiece));
			map = map & (map - 1);
		}
	}

	void Generator::addPawnMoves() {
		Bitmap posMap = board->positions[board->colour][PAWN];
		Bitmask endRow = pawnEndRows[board->colour];
		posMap &= ~endRow;  // pawns on the end row  are dealt with in addPromotionMoves()
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmask pseudoMoves = pseudoPawn(position);
			Bitmask pinMask = pinMasks[position];
			Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addKnightMoves() {
		Bitmap posMap = board->positions[board->colour][KNIGHT];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmask pseudoMoves = knightPseudoLookup[position];
			Bitmask pinMask = pinMasks[position];
			Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addBishopMoves() {
		Bitmap posMap = board->positions[board->colour][BISHOP];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmask pseudoMoves = pseudoBishop(position);
			Bitmask pinMask = pinMasks[position];
			Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addRookMoves() {
		Bitmap posMap = board->positions[board->colour][ROOK];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmask pseudoMoves = pseudoRook(position);
			Bitmask pinMask = pinMasks[position];
			Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}

	void Generator::addQueenMoves() {
		Bitmap posMap = board->positions[board->colour][QUEEN];
		while (posMap) {
			int position = getNextPosition(posMap);
			Bitmask pseudoMoves = pseudoQueen(position);
			Bitmask pinMask = pinMasks[position];
			Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
			addMoves(position, legalMoves, Flag::NONE, PAWN);
		}
	}
	
	void Generator::addKingMoves() {
		int position = getSinglePosition(board->positions[board->colour][KING]);
		Bitmask pseudoMoves = pseudoKing(position);
		printmap(pseudoMoves);
		Bitmask pinMask = pinMasks[position];
		Bitmask legalMoves = pseudoMoves & checkMask & pinMask & enemyEmptyMask;
		addMoves(position, legalMoves, Flag::NONE, PAWN);
	}

	void Generator::addCastleMoves() {}

	void Generator::addPromotionMoves() {}

	void Generator::addEnPassantMoves() {}

	Bitmask Generator::pseudoPawn(int pos) {
		Bitmask nall = ~board->all;
		Bitmask forwardMap = pawnPushLookup[board->colour][pos];
		forwardMap &= nall;  // can't step on another piece

		if (forwardMap & pawnDoubleStepRows[board->colour]) {
			forwardMap |= pawnPushLookup[board->colour][pos + pawnStepSizes[board->colour]] & nall;
		}
		
		Bitmask attackMap = pawnAttackLookup[board->colour][pos] & board->all;

		return forwardMap | attackMap;
	}
	Bitmask Generator::pseudoKnight(int pos) { return 0ULL; }
	Bitmask Generator::pseudoBishop(int pos) { 
		return bishopPseudoLookup[pos][board->all];
	}
	Bitmask Generator::pseudoRook(int pos) { 
		return rookPseudoLookup[pos][board->all];
	}
	Bitmask Generator::pseudoQueen(int pos) { 
		return pseudoBishop(pos) | pseudoRook(pos);
	}
	Bitmask Generator::pseudoKing(int pos) { 
		return kingPseudoLookup[pos];
	}

	void loadPawnPseudoPushMoves(Bitmask arr[2][64]) {
		for (int pos = 0; pos < 64; pos++) {
			Bitmap posMap = bitset[pos];
			arr[WHITE][pos] = posMap << 8;
			arr[BLACK][pos] = posMap >> 8;
		}
	}

	void loadPawnPseudoAttackMoves(Bitmask arr[2][64]) {
		for (int pos = 0; pos < 64; pos++) {
			int col = 7 - pos % 8; // a=0 ... h=7
			Bitmap posMap = bitset[pos];
			Bitmask whiteMask = 0, blackMask = 0;
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

	void loadKnightPseudoMoves(Bitmask arr[64]) {
		for (int pos = 0; pos < 64; pos++) {
			Bitmask neighbours = 0ULL;
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

	void loadKingPseudoMoves(Bitmask arr[64]) {
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
}
	
