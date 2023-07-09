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

	const int DEFAULT_TIME = 100.0; // milliseconds

	const size_t TRANSPOSITION_SIZE = 10000000;

	const int POSITIVE_INFINITY = 99999999;
	const int NEGATIVE_INFINITY = -POSITIVE_INFINITY;

	const int CHECKMATE_SCORE = -9999999;

	Bot::Bot(): tt(TRANSPOSITION_SIZE) {
		maxSearchTime = DEFAULT_TIME;
		nodes = 0;
	}

	Bot::Bot(double searchTime, bool quies) : tt(TRANSPOSITION_SIZE), runQuiescence(quies) {
		maxSearchTime = searchTime;
		nodes = 0;
	}

	Move Bot::search(Board board) {
		//tt.clear();

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

		if (board.colour == BLACK) bestEval = -bestEval;
		
		std::cout << "nodes: " << formatToCommas(std::to_string(nodes)) << std::endl;
		std::cout << "n/s: " << formatToCommas(std::to_string((int)(nodes/maxSearchTime * 1000))) << std::endl;
		std::cout << "value: " << formatToCommas(std::to_string(bestEval)) << std::endl;
		std::cout << "transpositions: " << formatToCommas(std::to_string(transposCount)) << std::endl;
		std::cout << "transpos storage used: " << tt.percentFull() << '%' << std::endl;

		return bestMove;
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
			if (runQuiescence) return quiescence(board, -beta, -alpha);
			else return evaluate(board);
		}

		MoveList moves(generator);
		

		if (moves.size() == 0) {
			if (generator.isCheck()) return CHECKMATE_SCORE - depth; // subtract depth to favour mates in shorter time spans
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

	int Bot::quiescence(Board& board, int alpha, int beta) {
		if (tt.contains(board.zobrist, 0, alpha, beta, true)) {
			transposCount++;
			return tt[board.zobrist].value;
		}

		int currentEval = evaluate(board);

		if (currentEval >= beta) {
			return beta;
		}
		if (currentEval > alpha) {
			alpha = currentEval;
		}

		MoveList moves(generator, true);

		order(board, moves);

		NodeType nodeType = NodeType::LOWER;

		for (Move move : moves) {
			nodes++;
			board.makeMove(move);
			int score = -quiescence(board, -beta, -alpha);
			board.unmakeMove(move);

			if (score >= beta) {
				nodeType = NodeType::UPPER;
				alpha = beta;
				break;
			}
			if (score > alpha) {
				nodeType = NodeType::LOWER;
				alpha = score;
			}
		}

		TTEntry entry = { board.zobrist, 0, alpha, nodeType, true };
		tt.replace(entry);

		return alpha;
	}

	bool Bot::shouldFinishSearch() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now() - searchStartTime).count() > maxSearchTime;
	}
}