#pragma once

#include "pieces.hpp"

#include <string>

namespace chess
{
	using Bitmap = uint64_t;  // for piece positions
	using Bitmask = uint64_t; // for bitmap modifying

	enum class Flag {
		NONE, PROMOTION, EN_PASSANT, CASTLE
	};

	static const char columnLetters[9] = "abcdefgh";

	const size_t maxMoveCount = 218; // maximum possible number of moves from any position

	struct Move {
	public:
		Bitmap start, end;
		Flag flag;
		Type promotionPiece;

		bool operator==(Move other);

		Move(Bitmap pStart, Bitmap pEnd);
		Move(Bitmap pStart, Bitmap pEnd, Flag flag, Type pPiece);
		Move();
		static Move promotion(Bitmap pStart, Bitmap pEnd, Type pPiece);
		static Move enPassant(Bitmap pStart, Bitmap pEnd);
		static Move castle(Bitmap pStart, Bitmap pEnd);

		std::string notate();

	};
}