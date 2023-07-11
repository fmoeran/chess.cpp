#include "game.hpp"

#include <string>
#include <iostream>

namespace chess
{
	std::string startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w QKqk - 0 1";
	//std::string startingFen = "3rr3/3k4/8/3K4/8/8/8/8 w - - 0 1";
	//std::string startingFen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
	//std::string startingFen = "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1";

	chess::Game::Game(sf::RenderWindow& window, float size, bool whiteAI, bool blackAI, bool debug, sf::Vector2f coords) {
		display = BoardDisplay(window, size, debug, coords);
		board = Board::fromFen(startingFen);
		generator = Generator(board);

		currentLegalMoves = MoveList();
		whiteIsAI = whiteAI;
		blackIsAI = blackAI;
		mousePressed = false;
		mouseReleased = false;
		isHolding = false;
	}


	chess::Game::Game(bool whiteAI, bool blackAI) {
		board = Board::fromFen(startingFen);
		generator = Generator(board);

		currentLegalMoves = MoveList();
		whiteIsAI = whiteAI;
		blackIsAI = blackAI;
		mousePressed = false;
		mouseReleased = false;
		isHolding = false;
	}

	chess::Game::Game() {
		whiteIsAI = false;
		blackIsAI = false;
		board = Board::fromFen(startingFen);
		generator = Generator(board);
		currentLegalMoves = MoveList();
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

			bool ended = move();
			if (ended) break;
		}
		board.print();
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
		if (pastMoves.empty()) return;
		board.unmakeMove(pastMoves.top());
		pastMoves.pop();
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

	void chess::Game::aiMove() {
		Move bestMove = bot.search(board);
		movePiece(bestMove);
	}

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
		currentLegalMoves = MoveList(generator);

	}

	void chess::Game::grabPosition(int pos) {
		Bitmap posmap = bitset[pos];
		// no friendly piece on that square
		if ((posmap & board.teamMaps[board.colour]) == 0) return;

		for (Type type = PAWN; type <= KING; type++) {
			if (board.positions[board.colour][type] & posmap) {
				holding = Piece(type, board.colour);
				pickedPosition = pos;
				isHolding = true;
				updateMoveHighlights();
				return;
			}
		}
	}

	void chess::Game::placeHolding(int pos) {
		Move move = generateMove(pickedPosition, pos);

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

	Move chess::Game::generateMove(int start, int end) {
		// default move
		Move move = makeMove(start, end);

		if (moveIsLegal(move)) {
			return move;
		}
		

		// castle
		if (bitset[start] & board.positions[board.colour][KING]) {
			move = makeCastle(start, end);
			if (moveIsLegal(move)) {
				return move;
			}
		}
		
		// pawn
		if (bitset[start] & board.positions[board.colour][PAWN]) {
			
			// en passant
			move = makeEnPassant(start, end);
			if (moveIsLegal(move)) {
				return move;
			}
			// promotion
			move = makePromotion(start, end, QUEEN);
			if (moveIsLegal(move)) {
				Piece promotionPiece = display.askUserForPromotionpiece(board.colour);
				return makePromotion(start, end, promotionPiece.type);
			}
		}

		return makeMove(start, end); // will be illegal
	}

	void chess::Game::movePiece(Move move) {
		if (!moveIsLegal(move)) {
			std::cout << "Illegal move attempted: " << notate(move) << std::endl;
			board.print();
			throw("Illegal move attempted");
		}
		board.makeMove(move);
		pastMoves.push(move);
		currentLegalMoves.clear();
	}

	void chess::Game::updateMoveHighlights() {
		resetMoveHighlights();
		for (Move& move : currentLegalMoves) {
			if (getStart(move) == pickedPosition) {
				display.highlightMap |= bitset[getEnd(move)];
			}
		}
	}

	void chess::Game::resetMoveHighlights() {
		display.highlightMap = 0ULL;
	}
}
