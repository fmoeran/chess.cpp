#include "move.hpp"
#include <iostream>

namespace chess
{
	// takes either knight, bishop, rook, queen and squashes it to the first 2 bits
	int pieceToPromotionCode(Type piece) {
		return piece - 1;
	}
	bool Move::operator==(Move other){
		return value == other.value;
	}

	Move::Move(int pStart, int pEnd){
		value = pStart | (pEnd << 6);
	}

	chess::Move::Move(int pStart, int pEnd, Flag flag, Type pPiece){
		value = pStart | (pEnd << 6) | ((int)flag << 12) | (pieceToPromotionCode(pPiece) << 14);
	}

	chess::Move::Move()
	{
		value = 0;
	}

	Move Move::promotion(int pStart, int pEnd, Type pPiece)
	{
		return Move(pStart, pEnd, Flag::PROMOTION, pPiece);
	}

	Move Move::enPassant(int pStart, int pEnd)
	{
		return Move(pStart, pEnd, Flag::EN_PASSANT, PAWN);
	}

	Move Move::castle(int pStart, int pEnd)
	{
		return Move(pStart, pEnd, Flag::CASTLE, PAWN);
	}

	int Move::start() {
		return value & 0b111111;;
	}

	int Move::end() {
		return (value >> 6) & 0b111111;
	}

	Flag Move::flag() {
		return (Flag)(value>>12 & 0b11);
	}

	Type Move::promotionPiece() {
		return ((value >> 14) & 0b11) + 1;
	}


	std::string Move::notate()
	{
		int startPos = start();
		int endPos = end();
		std::string starting = std::string({ columnLetters[7 - startPos % 8], (char)(startPos / 8 + 1 + '0') });
		std::string ending = std::string({ columnLetters[7 - endPos % 8], (char)(endPos / 8 + 1 + '0') });
		std::string promotion = "";
		if (flag() == Flag::PROMOTION) promotion += pieceLetters[promotionPiece()];
		return starting + ending + promotion;
	}

	
}
