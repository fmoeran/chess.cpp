#pragma once

#include "SFML/Graphics.hpp"

#include "display.hpp"
#include "search.hpp"
#include "generator.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "pieces.hpp"

#include <vector>

namespace chess
{
	enum class Result {
		WIN, LOSS, DRAW
	};

	class Game {
	public:
		Game(sf::RenderWindow& window, float size, bool whiteAI, bool blackAI, bool debug, sf::Vector2f coords);
		Game(bool whiteAI, bool blackAI);
		Game();
		

		Result run();
		bool running;
		
	private:
		Board board;
		BoardDisplay display;
		Generator generator;
		Bot bot;


		std::stack<Move> pastMoves;
		// white players are AI
		bool whiteIsAI, blackIsAI;
		// true on the fram that it occurs
		bool mousePressed, mouseReleased;
		// the current piece the player is holding
		Piece holding;
		bool isHolding;
		int pickedPosition;  // where holding was picked from

		// holds all the current legal moves for the current moving player
		MoveList currentLegalMoves;

		// handles input from user each frame
		void handleEvents();
		// undoes the last made move
		void undoMove();
		// updates the currentLegalMoves
		void updateCurrentMoves();
		// returns a legal Move class from a pickup and drop position
		// this requires currentLegalMoves to be updated
		Move generateMove(int start, int end);
		// attempts to move a piece through board.makeMove
		// if move is illegal it will throw an exception
		void movePiece(Move move);

		void updateMoveHighlights();
		void resetMoveHighlights();
		// activates holding, isHolding, pickedPosition
		void grabPosition(int pos);
		// places holding
		void placeHolding(int pos);

		bool moveIsLegal(Move move);

		// handles making a move
		// \return whether the player has any moves possible
		bool move();
		void playerMove();
		void aiMove();

		
	};
}