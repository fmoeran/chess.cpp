#include "game.hpp"

#include <string>
#include <iostream>

namespace chess
{



	chess::Game::Game(sf::RenderWindow& window, float size, bool whiteAI, bool blackAI, bool debug, sf::Vector2f coords) {
		display = BoardDisplay(window, size, debug, coords);
		generator = Generator(board);

		whiteIsAI = whiteAI;
		blackIsAI = blackAI;

		mousePressed = false;
		mouseReleased = false;
		isHolding = false;
	}


	chess::Game::Game(sf::RenderWindow& window, bool whiteAI, bool blackAI) {
		display = BoardDisplay(window, 90.0, false, sf::Vector2f(0, 0));
		generator = Generator(board);

		whiteIsAI = whiteAI;
		blackIsAI = blackAI;

		mousePressed = false;
		mouseReleased = false;
		isHolding = false;
	}

	chess::Game::Game() {
		whiteIsAI = false;
		blackIsAI = false;

		generator = Generator(board);

		mousePressed = false;
		mouseReleased = false;
		isHolding = false;
	}



	Result chess::Game::run() {
		running = true;
		while (running) {
			handleEvents();
			if (isHolding) display.updateScreen(board, holding, pickedPosition);
			else display.updateScreen(board);

			move();
		}


		return Result::WIN;
	}

	void chess::Game::handleEvents() {
		mousePressed = false;
		mouseReleased = false;
		sf::Event eventInput;
		while (display.window->pollEvent(eventInput)) {
			switch (eventInput.type) {
			case sf::Event::Closed:
				running = false;
			case sf::Event::KeyPressed:
				if (eventInput.key.code == sf::Keyboard::Escape) {
					running = false;
				}
				else if (eventInput.key.code == sf::Keyboard::Space) {
					undoMove();
				}
			case sf::Event::MouseButtonPressed:
				if (eventInput.mouseButton.button == sf::Mouse::Left) {
					mousePressed = true;
				}
			case sf::Event::MouseButtonReleased:
				if (eventInput.mouseButton.button == sf::Mouse::Left) {
					mouseReleased = true;
				}
			}
		}
	}

	void chess::Game::undoMove() {
		if (board.logs.size() == 0) return;
		board.unmakeMove();
		updateCurrentMoves();
		isHolding = false;
	}

	bool chess::Game::move() {
		if (currentLegalMoves.size() == 0) {
			updateCurrentMoves();
			// draw or loss
			if (currentLegalMoves.size() == 0) {
				return true;
			}
		}

		if ((board.colour == WHITE && whiteIsAI) || (board.colour == BLACK && blackIsAI)) {
			aiMove();
		}
		else {
			playerMove();
		}
		return false;
	}

	void chess::Game::aiMove() {}

	void chess::Game::playerMove() {
		if (!display.mouseIsOnBoard()) return;
		else if (mousePressed && !isHolding) { // pick up
			grabPosition(display.getMousePosition());
		}
		else if (mouseReleased && isHolding) { // drop
			placeHolding(display.getMousePosition());
		}
	}

	void chess::Game::updateCurrentMoves() {
		currentLegalMoves = generator.getLegalMoves();

	}

	void chess::Game::grabPosition(Bitmap posmap) {

		// no friendly piece on that square
		if ((posmap & board.teamMaps[board.colour]) == 0) return;

		for (Type type = PAWN; type <= KING; type++) {
			if (board.positions[board.colour][type] & posmap) {
				holding = Piece(type, board.colour);
				pickedPosition = posmap;
				isHolding = true;
				updateMoveHighlights();
				return;
			}
		}
	}

	void chess::Game::placeHolding(Bitmap posmap) {
		Move move = generateMove(pickedPosition, posmap);

		bool isLegal = std::find(currentLegalMoves.begin(), currentLegalMoves.end(), move) != currentLegalMoves.end();

		if (isLegal) {
			movePiece(move);
		}

		isHolding = false;

		resetMoveHighlights();
	}

	bool chess::Game::moveIsLegal(Move move) {
		return std::find(currentLegalMoves.begin(), currentLegalMoves.end(), move) != currentLegalMoves.end();
	}

	Move chess::Game::generateMove(Bitmap start, Bitmap end) {
		// default move
		Move move(start, end);

		if (moveIsLegal(move)) {
			return move;
		}

		// castle
		if (start & board.positions[board.colour][KING]) {
			move = Move::castle(start, end);
			if (moveIsLegal(move)) {
				return move;
			}
		}
		// pawn
		if (start & board.positions[board.colour][PAWN]) {
			// en passant
			move = Move::enPassant(start, end);
			if (moveIsLegal(move)) {
				return move;
			}
			// promotion
			move = Move::promotion(start, end, QUEEN);
			if (moveIsLegal(move)) {
				Piece promotionPiece = display.askUserForPromotionpiece(board.colour);
				return Move::promotion(start, end, promotionPiece.type);
			}
		}

		return Move(start, end); // will be illegal
	}

	void chess::Game::movePiece(Move move) {
		if (!moveIsLegal(move)) throw("Illegal move attempted");

		board.makeMove(move);
		currentLegalMoves.clear();
	}

	void chess::Game::updateMoveHighlights() {
		resetMoveHighlights();
		for (Move& move : currentLegalMoves) {
			if (move.start & pickedPosition) {
				display.highlightMap |= move.end;
			}
		}
	}

	void chess::Game::resetMoveHighlights() {
		display.highlightMap = 0ULL;
	}
}

/*int main() {
	Game game;
	
	Result res = game.run();
}*/