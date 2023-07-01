#include "move.hpp"
#include <iostream>

namespace chess
{
	// takes either knight, bishop, rook, queen and squashes it to the first 2 bits
	int pieceToPromotionCode(Type piece) {
		return piece - 1;
	}

	Move makeMove(int start, int end){
		return (Move)start | (end << 6);
	}

	Move makeMove(int start, int end, Flag flag, Type promotion) {
		return start | (end << 6) | ((int)flag << 12) | (pieceToPromotionCode(promotion) << 14);
	}

	Move makePromotion(int start, int end, Type piece)
	{
		return makeMove(start, end, Flag::PROMOTION, piece);
	}

	Move makeEnPassant(int start, int end)
	{
		return makeMove(start, end, Flag::EN_PASSANT, PAWN);
	}

	Move makeCastle(int start, int end)
	{
		return makeMove(start, end, Flag::CASTLE, PAWN);
	}

	int getStart(Move move) {
		return move & 0b111111;;
	}

	int getEnd(Move move) {
		return (move >> 6) & 0b111111;
	}

	Flag getFlag(Move move) {
		return (Flag)(move>>12 & 0b11);
	}

	Type getPromotion(Move move) {
		return ((move >> 14) & 0b11) + 1;
	}


	std::string notate(Move move)
	{
		int startPos = getStart(move);
		int endPos = getStart(move);
		std::string starting = std::string({ columnLetters[7 - startPos % 8], (char)(startPos / 8 + 1 + '0') });
		std::string ending = std::string({ columnLetters[7 - endPos % 8], (char)(endPos / 8 + 1 + '0') });
		std::string promotion = "";
		if (getFlag(move) == Flag::PROMOTION) promotion += pieceLetters[getPromotion(move)];
		return starting + ending + promotion;
	}

	
}
