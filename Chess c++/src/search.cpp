#include "search.hpp"
#include "evaluate.hpp"
#include "order.hpp"

#include <iostream>
#include <stdlib.h>
#include <chrono>

std::string formatCommas(std::string s) {
	int n = (int)s.size() - 3;
	if (s[0] == '-') n--;
	while (n > 0) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}


namespace chess
{
	const int MAX_SEARCH_DEPTH = 10000;

	const int DEFAULT_TIME = 1000.0; // milliseconds

	const int POSITIVE_INFINITY = 99999999;
	const int NEGATIVE_INFINITY = -POSITIVE_INFINITY;

	Bot::Bot() {
		maxSearchTime = DEFAULT_TIME;
		nodes = 0;
	}

	Bot::Bot(double searchTime) {
		maxSearchTime = searchTime;
		nodes = 0;
	}

	Move Bot::search(Board board) {
		searchStartTime = std::chrono::system_clock::now();

		generator = Generator(board);
		int searchDepth = 1;

		Move bestMove = NULL_MOVE;
		int bestEval = 0;
		
		for (; searchDepth < MAX_SEARCH_DEPTH; searchDepth++) {
			searchRoot(board, searchDepth);


			if (shouldFinishSearch()) {
				break;
			}

			if (bestRootMove != NULL_MOVE) {
				bestMove = bestRootMove;
				bestEval = bestRootEval;
			}

			std::cout << "depth: " << searchDepth << '\r';
			std::cout.flush();
		}
		std::cout << std::endl;
		
		std::cout << "nodes: " << formatCommas(std::to_string(nodes)) << std::endl;
		std::cout << "value: " << formatCommas(std::to_string(bestRootEval)) << std::endl;

		return bestRootMove;

	}

	void Bot::searchRoot(Board& board, int depth) {
		nodes = 0;
		MoveList moves(generator);
		order(board, moves);

		nodes += (int)moves.size();

		Move bestMove = NULL_MOVE;
		int bestScore = NEGATIVE_INFINITY;

		for (Move move : moves) {

			board.makeMove(move);
			int score = -negamax(board, depth - 1, NEGATIVE_INFINITY, -bestScore);
			board.unmakeMove(move);

			if (score > bestScore) {
				bestScore = score;
				bestMove = move;
			}
			if (shouldFinishSearch()) return;
		}
		bestRootMove = bestMove;
		bestRootEval = bestScore;

	}

	

	int Bot::negamax(Board& board, int depth, int alpha, int beta) {

		if (shouldFinishSearch()) return beta;

		if (depth == 0) {
			return evaluate(board);
		}

		MoveList moves(generator);
		

		if (moves.size() == 0) {
			if (generator.isCheck()) return NEGATIVE_INFINITY - depth; // subtract depth to favour mates in shorter time spans
			else return 0;
		}

		order(board, moves);

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

	bool Bot::shouldFinishSearch() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now() - searchStartTime).count() > maxSearchTime;
	}



}