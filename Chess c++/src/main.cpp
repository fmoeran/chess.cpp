#include "game.hpp"
#include <iostream>

using namespace chess;

int main() {
	Game game(false, true);
	Result result = game.run();
	std::cout << (int)result << std::endl;
}