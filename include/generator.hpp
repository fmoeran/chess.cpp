#pragma once

#include "bitboard.hpp"
#include "move.hpp"
#include "pieces.hpp"

#include <vector>
#include <memory>


namespace chess
{
	class Generator {
	public:
		Generator();
		Generator(Board& pboard);


		std::vector<Move> getLegalMoves();
		Board* board;
		
	private:
		std::vector<Move> moves;
		
		// every position that is either an enemy or empty
		Bitmask enemyEmptyMask;
		// positions of every piece that is currently checking the king
		Bitmask checkMask;
		// positions of every attacked position by the enemy
		Bitmask attackMask;
		// the positions every piece can move to whilst not opening a pin
		Bitmask pinMasks[64];

		void loadEnemyEmptyMask();
		void loadCheckMask();
		void loadAttackMask();
		void loadPinMasks();
		// used for checking when en passant pawns are taken
		bool isPinned(Bitmap posMap);

		// adding moves onto Generator::moves
		void addMoves(int position, Bitmap map, Flag flag, Type promotionPiece);
		
		void addPawnMoves();
		void addKnightMoves();
		void addBishopMoves();
		void addRookMoves();
		void addQueenMoves();
		void addKingMoves();

		void addCastleMoves();
		void addPromotionMoves();
		void addEnPassantMoves();

		// return bitmasks of all the pseudo legal move positions from an integer position
		Bitmask pseudoPawn(int pos);
		Bitmask pseudoKnight(int pos);
		Bitmask pseudoBishop(int pos);
		Bitmask pseudoRook(int pos);
		Bitmask pseudoQueen(int pos);
		Bitmask pseudoKing(int pos);
	};

}