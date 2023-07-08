#include "search.hpp"
#include "evaluate.hpp"
#include "order.hpp"

#include <iostream>
#include <stdlib.h>
#include <chrono>

std::string formatToCommas(std::string s) {
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

	const size_t TRANSPOSITION_SIZE = 10000000;

	const int POSITIVE_INFINITY = 99999999;
	const int NEGATIVE_INFINITY = -POSITIVE_INFINITY;

	Bot::Bot(): tt(TRANSPOSITION_SIZE) {
		maxSearchTime = DEFAULT_TIME;
		nodes = 0;
	}

	Bot::Bot(double searchTime) : tt(TRANSPOSITION_SIZE) {
		maxSearchTime = searchTime;
		nodes = 0;
	}

	Move Bot::search(Board board) {
		transposCount = 0;
		nodes = 0;
		searchStartTime = std::chrono::system_clock::now();

		generator = Generator(board);
		int searchDepth = 1;

		Move bestMove = NULL_MOVE;
		int bestEval = 0;
		
		for (; searchDepth < MAX_SEARCH_DEPTH; searchDepth++) {
			searchRoot(board, searchDepth);
			//std::cout << std::endl;

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
		
		std::cout << "nodes: " << formatToCommas(std::to_string(nodes)) << std::endl;
		std::cout << "value: " << formatToCommas(std::to_string(bestRootEval)) << std::endl;
		std::cout << "transpositions: " << formatToCommas(std::to_string(transposCount)) << std::endl;
		std::cout << "transpos storage used: " << tt.percentFull() << '%' << std::endl;

		return bestRootMove;

	}

	void Bot::searchRoot(Board& board, int depth) {
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
			//std::cout << bestScore << ' ';
			if (shouldFinishSearch()) return;
		}
		bestRootMove = bestMove;
		bestRootEval = bestScore;

	}

	

	int Bot::negamax(Board& board, int depth, int alpha, int beta) {

		if (tt.contains(board.zobrist, depth, alpha, beta)) {
			transposCount++;
			return tt[board.zobrist].value;
		}

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

		NodeType nodeType = NodeType::LOWER;

		int best = NEGATIVE_INFINITY;

		for (Move move : moves) {
			nodes++;
			board.makeMove(move);
			int score = -negamax(board, depth - 1, -beta, -alpha);
			board.unmakeMove(move);

			if (score >= beta) {
				nodeType = NodeType::UPPER;
				best = beta;
				break;
			}
			if (score > best) {
				nodeType = NodeType::LOWER;
				best = score;
				if (score > alpha) {
					alpha = score;
				}
			}
		}

		TTEntry entry = { board.zobrist, depth, best, nodeType, false };
		tt.replace(entry);
		return best;
	}

	bool Bot::shouldFinishSearch() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now() - searchStartTime).count() > maxSearchTime;
	}



}