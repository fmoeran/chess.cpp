#include "move.hpp"
#include <iostream>

namespace chess
{

	bool Move::operator==(Move other)
	{
		return start == other.start && end == other.end && flag == other.flag && promotionPiece == other.promotionPiece;
	}

	Move::Move(Bitmap pStart, Bitmap pEnd)
		: start(pStart), end(pEnd), flag(Flag::NONE), promotionPiece(PAWN) {}

	chess::Move::Move(Bitmap pStart, Bitmap pEnd, Flag flag, Type pPiece)
		: start(pStart), end(pEnd), flag(flag), promotionPiece(pPiece) {}

	chess::Move::Move()
	{
		start = 0;
		end = 0;
	}

	Move Move::promotion(Bitmap pStart, Bitmap pEnd, Type pPiece)
	{
		return Move(pStart, pEnd, Flag::PROMOTION, pPiece);
	}

	Move Move::enPassant(Bitmap pStart, Bitmap pEnd)
	{
		return Move(pStart, pEnd, Flag::EN_PASSANT, PAWN);
	}

	Move Move::castle(Bitmap pStart, Bitmap pEnd)
	{
		return Move(pStart, pEnd, Flag::CASTLE, PAWN);
	}

	std::string Move::notate()
	{
		Bitmask position = 0, startPos = 0, endPos = 0;
		while ((1ULL << position) < (1ULL << 63)) {
			if (start == (1ULL << position)) startPos = position;
			if (end == (1ULL << position)) endPos = position;
			position++;
		}
		std::string starting = std::string({ columnLetters[7 - startPos % 8], (char)(startPos / 8 + 1 + '0') });
		std::string ending = std::string({ columnLetters[7 - endPos % 8], (char)(endPos / 8 + 1 + '0') });
		std::string promotion = "";
		if (flag == Flag::PROMOTION) promotion += pieceLetters[promotionPiece];
		return starting + ending + promotion;
	}

	MoveList::MoveList() {
		count = 0u;
	}

	void MoveList::add(const Move& move) {
		moves[count] = move;
		count++;
	}

	MoveList::iterator MoveList::begin() {
		return &moves[0];
	}

	MoveList::iterator MoveList::end() {
		return &moves[count];
	}

	size_t MoveList::size() {
		return count;
	}

	void MoveList::clear() {
		count = 0u;
	}
}