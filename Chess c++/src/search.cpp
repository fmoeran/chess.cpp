#include "search.hpp"
#include "evaluate.hpp"
#include "order.hpp"

#include <iostream>
#include <stdlib.h>
#include <chrono>

std::string formatToCommas(std::string s) {
	int n = (int)s.size() - 3;
	int end = s[0] == '-' ? 1 : 0;
	while (n > end) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}


namespace chess
{
	const int MAX_SEARCH_DEPTH = 10000;

	const int DEFAULT_TIME = 1000; // milliseconds

	const size_t TRANSPOSITION_SIZE = 9999999;

	const int POSITIVE_INFINITY = 99999999;
	const int NEGATIVE_INFINITY = -POSITIVE_INFINITY;

	const int CHECKMATE_SCORE = -9999999;

	Bot::Bot(): tt(TRANSPOSITION_SIZE) {
		maxSearchTime = DEFAULT_TIME;
		nodes = 0;
		evalCount = 0;
		runQuiescence = false;
	}

	Bot::Bot(double searchTime, bool quies) : tt(TRANSPOSITION_SIZE), runQuiescence(quies) {
		maxSearchTime = searchTime;
		nodes = 0;
		evalCount = 0;
	}

	Move Bot::search(Board board) {
		//tt.clear();

		evalCount = 0;
		transposCount = 0;
		nodes = 0;
		searchStartTime = std::chrono::system_clock::now();

		generator = Generator(board);
		int searchDepth = 1;

		Move bestMove = NULL_MOVE;
		int bestEval = NEGATIVE_INFINITY;
		for (; searchDepth <= MAX_SEARCH_DEPTH; searchDepth++) {
			searchRoot(board, searchDepth);

			if (bestRootMove != NULL_MOVE) {
				bestMove = bestRootMove;
				bestEval = bestRootEval;
				std::cout << "depth: " << searchDepth << '\r';
				std::cout.flush();
			}

			if (shouldFinishSearch()) {
				break;
			}
		}
		std::cout << std::endl;

		if (board.colour == BLACK) bestEval = -bestEval;

		std::cout << "move: " << notate(bestMove) << std::endl;
		std::cout << "nodes: " << formatToCommas(std::to_string(nodes)) << std::endl;
		std::cout << "eval count: " << formatToCommas(std::to_string(evalCount)) << std::endl;
		std::cout << "n/s: " << formatToCommas(std::to_string((int)(nodes/maxSearchTime * 1000))) << std::endl;
		std::cout << "value: " << formatToCommas(std::to_string(bestEval)) << std::endl;
		std::cout << "transpositions: " << formatToCommas(std::to_string(transposCount)) << std::endl;
		std::cout << "transpos storage used: " << tt.percentFull() << '%' << std::endl;
		std::cout << std::endl;
		if (bestEval == POSITIVE_INFINITY || bestEval == NEGATIVE_INFINITY) std::cin.ignore();

		return bestMove;
	}

	void Bot::searchRoot(Board& board, int depth) {
		MoveList moves(generator);

		order(board, moves, tt);

		nodes += (int)moves.size();

		bestRootMove = NULL_MOVE;
		bestRootEval = NEGATIVE_INFINITY;

		for (Move move : moves) {

			board.makeMove(move);
			int score = -negamax(board, depth - 1, NEGATIVE_INFINITY, POSITIVE_INFINITY);
			board.unmakeMove(move);

			if (shouldFinishSearch()) return;

			if (score > bestRootEval) {
				bestRootEval = score;
				bestRootMove = move;
			}
		}

		tt.replace({ board.zobrist, depth, bestRootMove, bestRootEval, NodeType::EXACT});
	}

	

	int Bot::negamax(Board& board, int depth, int alpha, int beta) {
		
		if (tt.contains(board.zobrist, depth, alpha, beta)) {
			transposCount++;
			return tt[board.zobrist].value;
		}

		if (shouldFinishSearch()) return alpha;

		if (depth == 0) {
			int eval;
			if (runQuiescence) eval = quiescence(board, depth, alpha, beta);
			else {
				evalCount++;
				eval = evaluate(board);
			}
			tt.replace({ board.zobrist, depth, NULL_MOVE, eval, NodeType::EXACT});
			return eval;
		}

		MoveList moves(generator);
		
		if (moves.size() == 0) {
			if (generator.isCheck()) return CHECKMATE_SCORE - depth; // subtract depth to favour mates in shorter time spans
			else return 0;
		}

		order(board, moves, tt);

		NodeType nodeType = NodeType::LOWER;

		int bestEval = NEGATIVE_INFINITY;
		Move bestMove = NULL_MOVE;

		for (Move move : moves) {
			nodes++;
			board.makeMove(move);
			int score = -negamax(board, depth - 1, -beta, -alpha);
			board.unmakeMove(move);

			if (shouldFinishSearch()) return bestEval;

			if (score >= beta) {
				nodeType = NodeType::UPPER;
				bestEval = beta;
				bestMove = move;
				break;
			}
			if (score > bestEval) {
				nodeType = NodeType::EXACT;
				bestEval = score;
				bestMove = move;
				if (score > alpha) {
					alpha = score;
				}
			}
		}

		tt.replace({ board.zobrist, depth, bestMove, bestEval, nodeType});
		return bestEval;
	}

	int Bot::quiescence(Board& board, int depth, int alpha, int beta) {
		if (tt.contains(board.zobrist, depth, alpha, beta)) {
			transposCount++;
			return tt[board.zobrist].value;
		}

		evalCount++;
		int currentEval = evaluate(board);

		if (currentEval >= beta) {
			tt.replace({ board.zobrist, depth, NULL_MOVE, currentEval, NodeType::LOWER});
			return currentEval;
		}
		if (currentEval > alpha) {
			alpha = currentEval;
		}

		MoveList moves(generator, true);

		if (moves.size() == 0) {
			return currentEval;
		}

		order(board, moves, tt);

		NodeType nodeType = NodeType::LOWER;

		Move bestMove = NULL_MOVE;

		for (Move move : moves) {
			nodes++;
			board.makeMove(move);
			int score = -quiescence(board, depth, -beta, -alpha);
			board.unmakeMove(move);

			if (score >= beta) {
				nodeType = NodeType::UPPER;
				alpha = beta;
				break;
			}
			if (score > alpha) {
				nodeType = NodeType::EXACT;
				alpha = score;
				bestMove = move;
			}
		}

		TTEntry entry = { board.zobrist, depth, bestMove, alpha, nodeType };
		tt.replace(entry);

		return alpha;
	}

	bool Bot::shouldFinishSearch() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now() - searchStartTime).count() > maxSearchTime;
	}
}