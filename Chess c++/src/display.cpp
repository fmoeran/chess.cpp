#include "display.hpp"

#include <iostream>

using namespace chess;


void chess::BoardDisplay::init(sf::RenderWindow& pwindow, bool owned, float psquareSize, bool pdebug, sf::Vector2f pboardPosition) {
    if (!squareFont.loadFromFile("assets/fonts/sans.ttf")) throw("Could not load font");
    squareFontSize = (int)psquareSize / 5;
    // dimentions
    squareSize = psquareSize;
    boardPosition = pboardPosition;
    width = squareSize * 8;
    height = squareSize * 8;
    // window
    window = &pwindow;
    windowOwned = false;
    // board colours
    bgColour = sf::Color(0, 0, 0);
    lightColour = sf::Color(255, 255, 255);
    darkColour = sf::Color(112, 62, 4);
    // move highlights highlight
    highlightColour = sf::Color(184, 21, 0, 210U);
    highlightSquare = sf::RectangleShape(sf::Vector2f(squareSize, squareSize));
    highlightSquare.setFillColor(highlightColour);
 
    debug = pdebug;
    
    loadSquares();
    loadPieceTextures();
}

chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, bool pdebug, sf::Vector2f pboardPosition) {
    init(pwindow, false, psquareSize, pdebug, pboardPosition);
}
chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow, bool pdebug, sf::Vector2f pboardPosition) {
    init(pwindow, false, 90.0f, pdebug, pboardPosition);
}
chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, sf::Vector2f pboardPosition) {
    init(pwindow, false, psquareSize, false, pboardPosition);
}

chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow, float psquareSize, bool pdebug) {
    init(pwindow, false, psquareSize, pdebug, sf::Vector2f(0, 0));
}


chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow, float psquareSize) {
    init(pwindow, false, psquareSize, false, sf::Vector2f(0, 0));
}

chess::BoardDisplay::BoardDisplay(sf::RenderWindow& pwindow) {
    init(pwindow, false, 90.0f, false, sf::Vector2f(0, 0));
}
chess::BoardDisplay::BoardDisplay() {
    sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(720, 720), "Chess");
    init(*newWindow, true, 90.0f, false, sf::Vector2f(0, 0));
}


chess::BoardDisplay::~BoardDisplay() {
    if (windowOwned) {
        delete window;
    }
}

void chess::BoardDisplay::updateScreen(Board& board) {
    window->clear();

    displaySquares();
    displayPieces(board);

    window->display();
}

void chess::BoardDisplay::updateScreen(Board& board, Piece holding, Bitmap pickedPosition){
    window->clear();

    displaySquares();
    displayMoves();
    displayPieces(board);
    displayHolding(holding, pickedPosition);

    window->display();
}

void chess::BoardDisplay::displaySquares() {
    for (int i = 0; i < 64; i++) {
        displaySquare(i);
    }
}

void chess::BoardDisplay::displayMoves() {
    int position;
    Bitmap positions = highlightMap;
    while (positions != 0) {
        position = getNextPosition(positions);
        int row = 7 - position / 8, col = 7 - position % 8;
        highlightSquare.setPosition(col * squareSize + boardPosition.x, row * squareSize + boardPosition.y);
        window->draw(highlightSquare);
    }
}

void chess::BoardDisplay::displaySquare(int ind) {
    window->draw(squares[ind]);
    if (debug) window->draw(debugNums[ind]);
}


void chess::BoardDisplay::displayPieces(Board& board) {
    for (int pieceID = 0; pieceID < 12; pieceID++) {
        Colour colour = pieceID / 6;
        Type type = pieceID % 6;
        Bitmap pieceMap = board.positions[colour][type];
        int index;

        while ((index = getNextPosition(pieceMap)) != -1) {
            int position = 63 - index; // position relative to top left of screen
            int row = position / 8, col = position % 8;
            sf::Sprite sprite(pieceTextures[pieceID]);
            sprite.setPosition(boardPosition.x + col * squareSize, boardPosition.y + row * squareSize);
            window->draw(sprite);
        }
    }
}

void chess::BoardDisplay::displayHolding(Piece holding, Bitmap pickedPosition) {
    int intPos = getSinglePosition(pickedPosition);
    displaySquare(63 - intPos);
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    sf::Sprite sprite(pieceTextures[holding.getId()]);

    sprite.setPosition(mousePos.x - squareSize/2, mousePos.y - squareSize/2);
    window->draw(sprite);
}


void chess::BoardDisplay::loadSquares() {
    squares.resize(64);
    debugNums.resize(64);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            sf::Color colour = lightColour;
            sf::Color dcolour = darkColour;
            if ((row + col) % 2 == 1) {
                std::swap(colour, dcolour);
            }
            int ind = row * 8 + col;
            // square
            squares[ind] = sf::RectangleShape(sf::Vector2f(squareSize, squareSize));
            squares[ind].setPosition(boardPosition.x + col * squareSize, boardPosition.y + row * squareSize);
            squares[ind].setFillColor(colour);
            // debug nums
            debugNums[ind].setFont(squareFont);
            debugNums[ind].setCharacterSize(squareFontSize);
            debugNums[ind].setString(std::to_string(63 - ind));
            debugNums[ind].setPosition(squares[ind].getPosition());
            debugNums[ind].setFillColor(dcolour);

        }
    }
}

void chess::BoardDisplay::loadPieceTextures() {
    pieceTextures.resize(12);
    if (!pieceTextures[Piece(PAWN, WHITE).getId()].loadFromFile("assets/images/wP.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(KNIGHT, WHITE).getId()].loadFromFile("assets/images/wN.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(BISHOP, WHITE).getId()].loadFromFile("assets/images/wB.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(ROOK, WHITE).getId()].loadFromFile("assets/images/wR.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(QUEEN, WHITE).getId()].loadFromFile("assets/images/wQ.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(KING, WHITE).getId()].loadFromFile("assets/images/wK.png")) throw("Could not load piece texture");
    
    if (!pieceTextures[Piece(PAWN, BLACK).getId()].loadFromFile("assets/images/bP.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(KNIGHT, BLACK).getId()].loadFromFile("assets/images/bN.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(BISHOP, BLACK).getId()].loadFromFile("assets/images/bB.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(ROOK, BLACK).getId()].loadFromFile("assets/images/bR.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(QUEEN, BLACK).getId()].loadFromFile("assets/images/bQ.png")) throw("Could not load piece texture");
    if (!pieceTextures[Piece(KING, BLACK).getId()].loadFromFile("assets/images/bK.png")) throw("Could not load piece texture");
}

bool chess::BoardDisplay::mouseIsOnBoard() {
    return true;
}

Piece chess::BoardDisplay::askUserForPromotionpiece(Colour colour)
{
    return Piece(QUEEN, colour);
}

Bitmap chess::BoardDisplay::getMousePosition() {
    sf::Vector2i position = sf::Mouse::getPosition(*window);
    int row = (position.y - (int)boardPosition.y) / squareSize;
    int col = (position.x - (int)boardPosition.x) / squareSize;
    int intPos = row * 8 + col;
    return bitset[63 - intPos];
}
