#include "transposition.hpp"
#include <iostream>


namespace chess
{

	TranspositionTable::TranspositionTable(size_t tableSize) {
		size = tableSize;
		table = new TTEntry[size];
		std::memset(table, 0, sizeof(TTEntry) * size);
	}

	TranspositionTable::~TranspositionTable() {
		delete[] table;
	}

	TTEntry& TranspositionTable::operator[](Zobrist zobrist) {
		size_t ind = zobrist % size;
		return table[ind];
	}

	bool TranspositionTable::contains(Zobrist zobrist, int depth, int alpha, int beta, bool inQuies) {
		TTEntry entry = table[zobrist % size];
		if (entry.zobrist != zobrist || entry.depth < depth ||
			(entry.nodeType == NodeType::LOWER && entry.value < beta) ||
			(entry.nodeType == NodeType::UPPER && entry.value > alpha) ||
			(entry.inQuies && !inQuies))
			return false;
		return true;
	}

	void TranspositionTable::replace(TTEntry entry) {
		(*this)[entry.zobrist] = entry;
		return;
		if ((*this)[entry.zobrist].depth <= entry.depth) {
			(*this)[entry.zobrist] = entry;
		}
	}

	void TranspositionTable::clear() {
		std::memset(table, 0, sizeof(TTEntry) * size);
	}

	double TranspositionTable::percentFull() {
		TTEntry empty;
		std::memset(&empty, 0, sizeof(TTEntry));
		size_t count = 0;
		for (size_t i = 0; i < size; i++) {
			if (std::memcmp(&table[i], &empty, sizeof(TTEntry))) count++;
		}
		return (double)count / (double)size * 100.0;
	}

}