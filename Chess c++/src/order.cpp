#include "order.hpp"
#include "move.hpp"
#include "evaluate.hpp"
#include <algorithm>
#include <iostream>

namespace chess
{
	int estimateValue(const Board& board, Move move, TranspositionTable& tt) {
		int value = 0;
		bool capture = bitset[getEnd(move)] & board.teamMaps[!board.colour];
		if (capture) {
			Type startPiece, endPiece;
			for (Type piece = PAWN; piece <= KING; piece++) {
				if (bitset[getStart(move)] & board.positions[board.colour][piece]) startPiece = piece;
				if (bitset[getEnd(move)] & board.positions[!board.colour][piece]) endPiece = piece;
			}
			value += pieceWorths[endPiece] - pieceWorths[startPiece] / 10;
		}
		
		if (getFlag(move) == Flag::PROMOTION) {
			value += pieceWorths[getPromotion(move)];
		}

		Move hashMove = tt[board.zobrist].move;
		if (hashMove == move) {
			value += 10000;
		}

		return value;
	}


	void order(const Board& board, MoveList& moves, TranspositionTable& tt) {
		std::vector<std::pair<int, Move> > moveValuePairs;
		moveValuePairs.reserve(moves.size());
		for (Move move : moves) moveValuePairs.push_back({ -estimateValue(board, move, tt), move });

		std::sort(moveValuePairs.begin(), moveValuePairs.end());

		moves.clear();
		for (auto [value, move] : moveValuePairs) {
			moves.add(move);
		}
	}
}