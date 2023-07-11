#pragma once

#include "bitboard.hpp"

namespace chess
{
	enum class NodeType {
		EXACT,
		UPPER,
		LOWER
	};

	struct TTEntry {
		Zobrist zobrist;
		int depth;
		Move move;
		int value;
		NodeType nodeType;
	};

	struct TranspositionTable {
	public:
		TranspositionTable(size_t tableSize);
		~TranspositionTable();

		TTEntry& operator[](Zobrist zobrist);

		bool contains(Zobrist zobrist, int depth, int alpha, int beta);

		void replace(TTEntry entry);

		void clear();

		double percentFull();
	private:
		size_t size;
		TTEntry* table;
	};
}

