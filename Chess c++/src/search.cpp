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

void printLoadingBar(int current, int total, int width) {
	char filler = 219;
	std::cout << "[";
	float progress = (float)current / (float)total;
	int pos = width * progress;
	for (int i = 0; i < width; i++) {
		if (i <= pos) std::cout << filler;
		//else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "]";
	std::cout << int(progress * 100) << "%\r";
	std::cout.flush();
	
}


namespace chess
{
	const int DEFAULT_DEPTH = 6;

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
		auto t0 = std::chrono::high_resolution_clock::now();

		generator = Generator(board);

		MoveList moves(generator);
		order(board, moves);


		nodes += (int)moves.size();

		Move bestMove = NULL_MOVE;

		int alpha = NEGATIVE_INFINITY;
		int beta = POSITIVE_INFINITY;
		int i = 0;
		for (Move move : moves) {
			printLoadingBar(i, moves.size(), 40);
			i++;

			board.makeMove(move);
			int score = -negamax(board, defaultDepth - 1, -beta, -alpha);
			board.unmakeMove(move);

			if (score > alpha) {
				alpha = score;
				bestMove = move;
			}

		}
		printLoadingBar(i, moves.size(), 40);
		std::cout << std::endl;

		auto t1 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
		double elapsed = (double)duration.count() / 1000;

		std::cout << "nodes: " << formatCommas(std::to_string(nodes)) << std::endl;
		std::cout << "time: " << formatCommas(std::to_string((int)elapsed)) << "s" << std::endl;
		std::cout << "value: " << formatCommas(std::to_string(alpha)) << std::endl;

		return bestMove;

	}

	int Bot::negamax(Board& board, int depth, int alpha, int beta) {

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





}