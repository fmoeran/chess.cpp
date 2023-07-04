#pragma once

#include "move.hpp"
#include "bitboard.hpp"
#include "generator.hpp"


namespace chess 
{
	class Bot {
	public:
		Bot();
		Bot(int depth);

		// finds the best move in a given position
		Move search(Board board);

	private:
		Generator generator;
		int defaultDepth;
		int nodes;

		int negamax(Board& board, int depth, int alpha, int beta);

	};
}