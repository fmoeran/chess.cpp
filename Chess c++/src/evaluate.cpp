#include "evaluate.hpp"

#include <bit>

namespace chess
{
	const int pieceWorths[6] = {
		100, // pawm
		300, // knight
		350, // bishop
		500, // rook
		900, // queen
		0    // king
	};


	int evaluate(const Board& board)
	{
		int scoreSum = 0;

		for (Type piece = PAWN; piece <= KING; piece++) {
			scoreSum += std::popcount(board.positions[WHITE][piece]) * pieceWorths[piece];
			scoreSum -= std::popcount(board.positions[BLACK][piece]) * pieceWorths[piece];
		}

		return scoreSum;

	}
}

