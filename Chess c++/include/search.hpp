#pragma once

#include "move.hpp"
#include "bitboard.hpp"
#include "generator.hpp"
#include "transposition.hpp"

#include <chrono>


namespace chess 
{
	class Bot {
	public:
		Bot();
		Bot(double searchTime);

		// finds the best move in a given position
		// uses iterative deepening
		Move search(Board board);

	private:
		Move bestRootMove;
		int bestRootEval;
		TranspositionTable tt;
		int transposCount;
		

		Generator generator;
		int nodes;
		// stored in milliseconds
		double maxSearchTime;
		// time that a search started
		std::chrono::system_clock::time_point searchStartTime;

		bool shouldFinishSearch();

		// updates bestRoot move and eval
		// calles negamax up to a certain depth
		void searchRoot(Board& board, int depth);

		int negamax(Board& board, int depth, int alpha, int beta);

	};
}