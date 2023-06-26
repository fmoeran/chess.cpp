#pragma once

#include "move.hpp"

#include <vector>
#include <string>
#include <array>

namespace chess
{
	const Bitmap bitset[64] = {
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 
		0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 
		0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
		0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 
		0x100000000, 0x200000000, 0x400000000, 0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
		0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000, 0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
		0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000, 
		0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
};

	// prints a bitmap
	void printmap(Bitmap bitmap); 
	// retrieves the integer position of the front position on a bitmap
	// returns -1 when there is no set bit
	int getSinglePosition(Bitmap bitmap); 
	// retrieves the int position of the front position on a bitmap
	// also removes that position from the bitmap
	// returns -1 when there are no set bits
	int getNextPosition(Bitmap& bitmap);
	
	using CastleRules = std::array<bool, 2>;

	struct BoardState {
		Bitmap epMap;
		bool wlc, wrc, blc, brc; // castle rights
		int moveCount, halfMoveCount;
		Colour colour;
	};

	struct PieceInfo {
		Piece piece;
		Bitmap mapPosition;
		int intPosition;
	};

	struct Log {
		BoardState info;
		std::vector<PieceInfo> pieceInfo;

		Log(BoardState boardInfo);

		void addPiece(Piece piece, Bitmap mapPosition);

	};

	struct Board {
	public:
		Bitmap positions[2][6]; //arrays of bitmaps for each team and piece
		Bitmap teamMaps[2];     // arrays of bitmaps for whole teams
		Bitmap all;             // bitmap for every piece

		Bitmap epMap; // bitmap for takeable en passant position

		// bools for castling rights
		CastleRules rightCastles;
		CastleRules leftCastles;

		int currentMove;
		int halfMoves;
		Colour colour;

		// stores a list of piece positions that will be toggled when a move is made.
		// altered and reset each make_move call
		std::vector<std::pair<Piece, Bitmap>> currentlyAltering;

		std::vector<Log> logs;

		Board(Bitmap wp, Bitmap wn, Bitmap wb, Bitmap wr, Bitmap wq, Bitmap wk, Bitmap bp, Bitmap bn, Bitmap bb, Bitmap br, Bitmap bq, Bitmap bk, Bitmap ep,
		      bool wlc, bool wrc, bool blc, bool brc, int move_count, int hm, Colour colour);
		Board();

		// returns a fully initialied instance of a Board from a FEN string
		static Board fromFen(std::string fen); 

		std::string toString();

		void print();

		// makes a move on the board and updates logic (e.g. currentMove)
		// NOTE this does not take legality into account
		void makeMove(Move move);

		void unmakeMove();

	private:
		void setPositions(Bitmap wp, Bitmap wn, Bitmap wb, Bitmap wr, Bitmap wq, Bitmap wk, Bitmap bp, Bitmap bn, Bitmap bb, Bitmap br, Bitmap bq, Bitmap bk);
		void setGameState(Bitmap ep, bool wlc, bool wrc, bool blc, bool brc, int moveCount, int hm, Colour clr);
		BoardState& getGameState();

		// moving 1 friendly piece to a different square.
		// does not affect the opposing team
		void movePieceDefault(Bitmap start, Bitmap end, Type pieceType);
		void movePieceEp(Bitmap start, Bitmap end);
		void movePiecePromotion(Bitmap start, Bitmap end, Type promotionType);
		void movePieceCastle(Bitmap start, Bitmap end);
		// removes an enemy piece from the board
		void takePiece(Bitmap position, Type pieceType);
		// updates Board::epMap after a pawn has been moved
		void updateEpMap(Bitmap start, Bitmap end);
		// increments each game state variable after one ply (move)
		void incrementGameState();
		// updates Board::teamMaps and Board::all
		void updateTeamPositions();
		// removes a castling right when a rook is moved
		void removeSingleCastle(Bitmap rookPosition, Colour clr);

	};

}