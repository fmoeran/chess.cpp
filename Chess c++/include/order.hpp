#pragma once

#include "generator.hpp"
#include "bitboard.hpp"


namespace chess {
	// sorts moves to speed up alpha beta pruning
	void order(const Board& board, MoveList& moves);
}