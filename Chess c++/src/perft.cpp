#include "move.hpp"
#include "bitboard.hpp"
#include "generator.hpp"

#include <chrono>
#include <vector>
#include <iostream>
#include <string>

std::string formatCommas(std::string s) {
	int n = s.size() - 3;
	while (n > 0) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}


using namespace chess;

Board board;
Generator generator(board);

int perft(int depth, bool printMoves = true) {
	std::vector<Move> moves = generator.getLegalMoves();
	
	if (depth == 1) return (int)moves.size();
	
	int nodeCount = 0;
	
	for (Move move : moves) {
		board.makeMove(move);
		
		int count = perft(depth - 1, false);;
		nodeCount += count;

		if (printMoves) std::cout << move.notate() << ": " << formatCommas(std::to_string(count)) << std::endl;

		board.unmakeMove();
	}
	return nodeCount;
}

int main() {

#ifdef NDEBUG
	auto t0 = std::chrono::high_resolution_clock::now();

	int depth = 6;
	int nodes = perft(depth);
	auto t1 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
	double elapsed = (double)duration.count() / 1000;
	std::cout << "Finished in " << elapsed << "s" << std::endl;
	std::cout << formatCommas(std::to_string((int)((double)nodes / elapsed))) << " n/s" << std::endl;
	std::cout << formatCommas(std::to_string(nodes)) << " nodes" << std::endl;
#else
	std::cout << "Please set project to release mode" << std::endl;
#endif
}