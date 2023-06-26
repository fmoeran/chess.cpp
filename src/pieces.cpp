#include "pieces.hpp"

using namespace chess;

char chess::getChar(Type type, Colour colour)
{
	Piece piece(type, colour);
	return piece.getChar();
}


Piece::Piece(Type pType, Colour pColour) : type(pType), colour(pColour) {}

chess::Piece::Piece()
{
	type = PAWN;
	colour = WHITE;
}

bool Piece::operator==(Piece other) { return type == other.type && colour == other.colour; }

char Piece::getChar() {
	char letter = pieceLetters[type];
	if (colour == PieceColour::BLACK) letter += 'a'-'A';
	return letter;
}

int Piece::getId() {
	return type + colour * 6;
}


