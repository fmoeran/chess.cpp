#include "game.hpp"
#include <iostream>

using namespace chess;

int main() {
	Game game(true, true);
	Result result = game.run();
	std::cout << (int)result << std::endl;
}