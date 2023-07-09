#include "order.hpp"
#include "move.hpp"
#include "evaluate.hpp"
#include <algorithm>
#include <iostream>

namespace chess
{
	int estimateValue(const Board& board, Move move) {
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

		return value;
	}


	void order(const Board& board, MoveList& moves) {
		std::vector<std::pair<Move, int> > moveValuePairs;
		moveValuePairs.reserve(moves.size());
		for (Move move : moves) moveValuePairs.push_back({ move, -estimateValue(board, move) });

		std::sort(moveValuePairs.begin(), moveValuePairs.end());

		moves.clear();
		for (auto [move, value] : moveValuePairs) {
			moves.add(move);
		}
	}
}