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
		int value;
		NodeType nodeType;
		bool inQuies;
	};

	struct TranspositionTable {
	public:
		TranspositionTable(size_t tableSize);
		~TranspositionTable();

		TTEntry& operator[](Zobrist zobrist);

		bool contains(Zobrist zobrist, int depth, int alpha, int beta, bool inQuies = false);

		void replace(TTEntry entry);

		double percentFull();
	private:
		size_t size;
		TTEntry* table;
	};
}

