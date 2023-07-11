#pragma once

#include "generator.hpp"
#include "bitboard.hpp"
#include "transposition.hpp"


namespace chess {
	// sorts moves to speed up alpha beta pruning
	void order(const Board& board, MoveList& moves, TranspositionTable& tt);
}