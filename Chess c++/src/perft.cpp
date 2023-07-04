#include "move.hpp"
#include "bitboard.hpp"
#include "generator.hpp"

#include <chrono>
#include <vector>
#include <iostream>
#include <string>

using namespace chess;

struct Test {
	std::string fen;
	int depth;
	int expected;
};

Test tests[] = {
	{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRwKQkq - 0 1", 5, 4865609},
	{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4, 4085603},
	{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083},
	{"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292},
	{"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 5, 89941194}
};

Test startpos6 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRwKQkq - 0 1", 6, 119060324 };
Test startpos7 = { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRwKQkq - 0 1", 7, 3195901860 };



Board board;
Generator generator(board);


std::string formatCommas(std::string s) {
	int n = s.size() - 3;
	while (n > 0) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}

int perft(int depth, bool printMoves = true) {
	//MoveList moveList;
	//delete moveList;
	if (depth == 1) return (int)MoveList(generator).size();
	
	int nodeCount = 0;
	
	for (Move move : MoveList(generator)) {
		board.makeMove(move);
		
		int count = perft(depth - 1, false);
		nodeCount += count;

		if (printMoves) std::cout << notate(move) << ": " << formatCommas(std::to_string(count)) << std::endl;

		board.unmakeMove(move);
	}
	return nodeCount;
}

void runPerft(Test test, bool printMoves=true) {
	auto t0 = std::chrono::high_resolution_clock::now();
	int depth = test.depth;
	board = Board::fromFen(test.fen);

	int nodes = perft(depth, printMoves);

	auto t1 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
	double elapsed = (double)duration.count() / 1000;

	std::cout << elapsed << "s ";
	std::cout << formatCommas(std::to_string((int)((double)nodes / elapsed))) << " n/s ";
	if (nodes == test.expected) std::cout << "PASSED ";
	else {
		std::cout << "FAILED ";
		std::cout << "GOT " << formatCommas(std::to_string(nodes)) << " nodes ";
		std::cout << "EXPECTED " << formatCommas(std::to_string(test.expected));
	}
	std::cout << std::endl;
}

int main() {
	//for (Test test : tests) runPerft(test, false);
	runPerft(startpos6);
}