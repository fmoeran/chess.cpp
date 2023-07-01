#pragma once

#include "pieces.hpp"

#include <memory>
#include <string>

namespace chess
{
	using Bitmap = uint64_t;  // for piece positions

	enum class Flag {
		NONE=0, PROMOTION=1, EN_PASSANT=2, CASTLE=3
	};

	static const char columnLetters[9] = "abcdefgh";

	const size_t maxMoveCount = 218; // maximum possible number of moves from any position



	// move is stored in 16 bits of Move::value
	// 0-5 start position
	// 6-11 end position
	// 12-13 flag
	// 14-15 piece: knight(0), bishop(1), rook(2), queen(3)
	using Move = int;

	Move makeMove(int start, int end);
	Move makeMove(int start, int end, Flag flag, Type promotion);

	Move makePromotion(int start, int end, Type piece);
	Move makeEnPassant(int start, int end);
	Move makeCastle(int start, int end);

	int getStart(Move move);
	int getEnd(Move move);
	Flag getFlag(Move move);
	Type getPromotion(Move move);

	std::string notate(Move move);

	
	/*struct Move {
	public:
		int value;
		bool operator==(Move other);

		Move(int pStart, int pEnd);
		Move(int pStart, int pEnd, Flag flag, Type pPiece);
		Move();
		static Move promotion(int pStart, int pEnd, Type pPiece);
		static Move enPassant(int pStart, int pEnd);
		static Move castle(int pStart, int pEnd);

		int start();
		int end();
		Flag flag();
		Type promotionPiece();

		std::string notate();
	};*/

}