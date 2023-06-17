#pragma once
#include "bitboard.hpp"

#include <SFML/Graphics.hpp>


namespace chess
{
	class BoardDisplay {
	public:
		BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, bool pdebug, sf::Vector2f pboardPosition);
		BoardDisplay(sf::RenderWindow& pwindow, bool pdebug, sf::Vector2f pboardPosition);
		BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, sf::Vector2f pboardPosition);
		BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, bool pdebug);
		BoardDisplay(sf::RenderWindow& pwindow, float psquareSize);
		BoardDisplay(sf::RenderWindow& pwindow);
		BoardDisplay();

		~BoardDisplay();

		void updateScreen(Board& board);
		void updateScreen(Board& board, Piece holding, Bitmap pickedPosition);

		// retrieves the bitmap position of the current mouse position
		// mouse must be on the board for this to work
		Bitmap getMousePosition();

		bool mouseIsOnBoard();

		// gets the promotion piece Type wanted from the user
		Piece askUserForPromotionpiece(Colour colour);

		// window that the board is rendered on
		sf::RenderWindow* window;
		// map of positions to highlight
		Bitmap highlightMap;

	private:
		// initializer called by each constructor
		void init(sf::RenderWindow& pwindow, bool owned, float psquareSize, bool pdebug, sf::Vector2f pboardPosition);


		// font used for debug numbers
		sf::Font squareFont;
		int squareFontSize;

		// width and height of one square on the board
		float squareSize;
		// position of board in BoardDisplay::window
		sf::Vector2f boardPosition;
		// screen width
		float width;
		// screen height
		float height;
		
		// whether the window should be cleared and updated by this class
		bool windowOwned;

		// colours of the squares
		sf::Color lightColour, darkColour;
		// colour of the background of the window
		sf::Color bgColour;
		// colour of a square when it is highlighted for movement
		sf::Color highlightColour;
		// square used to show all move highlights
		sf::RectangleShape highlightSquare;

		// whether we will display debugNums
		bool debug;
		// array holding every square texture
		std::vector<sf::RectangleShape> squares;
		// array holding every debug number texture for each square
		std::vector<sf::Text> debugNums;
		// sprite for every piece type
		std::vector<sf::Texture> pieceTextures;

		// displays the square at squares[ind]
		void displaySquare(int ind);
		// displays all squares
		void displaySquares();
		// displays all possible moves using highlightPositions
		void displayMoves();
		// displays the current held piece that was picked up from pickedPosition
		void displayHolding(Piece holding, Bitmap pickedPosition);
		// retrieves the board row and col from a pixel coordinate
		void getMousePosition(sf::Vector2f coord);
		// displays all the pieces in a given board
		void displayPieces(Board& board);
		// cacheing squares and their debug nums
		void loadSquares();
		// cacheing images of pieces
		void loadPieceTextures();
	};


}
