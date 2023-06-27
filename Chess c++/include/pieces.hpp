#pragma once

namespace chess
{
	using Colour = bool;
	using Type = unsigned char;
	static char pieceLetters[7] = "PNBRQK";

	const enum PieceType {
		PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5
	};

	const enum PieceColour {
		WHITE = 0, BLACK = 1
	};

	char getChar(Type type, Colour colour);

	struct Piece {
	public:
		Type type;
		Colour colour;

		Piece(Type pType, Colour pColour);
		Piece();
		bool operator==(Piece other);
		char getChar();
		int getId();
	};

}