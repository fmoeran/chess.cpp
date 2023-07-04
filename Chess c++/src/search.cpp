#include "search.hpp"
#include "evaluate.hpp"

#include <iostream>

std::string formatCommas(std::string s) {
	int n = s.size() - 3;
	while (n > 0) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}

namespace chess
{
	const int DEFAULT_DEPTH = 7;

	const int POSITIVE_INFINITY = 99999999;
	const int NEGATIVE_INFINITY = -POSITIVE_INFINITY;

	Bot::Bot() {
		defaultDepth = DEFAULT_DEPTH;
		nodes = 0;
	}

	Bot::Bot(int depth) {
		defaultDepth = depth;
		nodes = 0;
	}

	Move Bot::search(Board board) {
		generator = Generator(board);

		MoveList moves(generator);

		nodes += moves.size();

		Move bestMove = NULL_MOVE;

		int alpha = NEGATIVE_INFINITY;
		int beta = POSITIVE_INFINITY;

		for (Move move : moves) {
			board.makeMove(move);
			int score = -negamax(board, defaultDepth - 1, -beta, -alpha);
			board.unmakeMove(move);

			if (score > alpha) {
				alpha = score;
				bestMove = move;
			}

		}

		std::cout << "nodes: " << formatCommas(std::to_string(nodes)) << std::endl;
		std::cout << "value: " << formatCommas(std::to_string(alpha)) << std::endl;

		return bestMove;

	}

	int Bot::negamax(Board& board, int depth, int alpha, int beta) {

		if (depth == 0) {
			return evaluate(board);
		}

		MoveList moves(generator);

		if (moves.size() == 0) {
			if (generator.isCheck()) return NEGATIVE_INFINITY;
			else return 0;
		}

		int best = NEGATIVE_INFINITY;

		for (Move move : moves) {
			nodes++;
			board.makeMove(move);
			int score = -negamax(board, depth - 1, -beta, -alpha);
			board.unmakeMove(move);

			if (score >= beta) {
				return beta;
			}
			if (score > best) {
				best = score;
				if (score > alpha) {
					alpha = score;
				}
			}
		}
		return best;
	}





}