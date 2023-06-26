#pragma once
#include "move.hpp"

#include <vector>

namespace chess
{
	struct MagicLookup {
	public:
		MagicLookup();
		// \param sq: the square the magic is for
		// \param bshp: whether the piece is a bishop (otherwise a rook)
		MagicLookup(int sq, bool bshp);
		// gets the pseudo move mask for the piece when facing allBoard
		// \param allBoard: positions of every piece on the board
		// \return mask of pseudo move positions that can be made
		Bitmask operator[] (Bitmask allBoard);
	private:
		int square;      // square on the boardthe lookup is for 
		bool isBishop;   // True bishop; False rook
		Bitmask mask;    // the positions that the piece would see without any enemies on the board
		Bitmask magic;   // magic number to transform the mask with a given blocker board
		int numBits;     // number of bits in mask
		std::vector<Bitmask> lookup;

	};


}