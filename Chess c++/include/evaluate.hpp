#pragma once

#include "bitboard.hpp"

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

	int evaluate(const Board& board);
}