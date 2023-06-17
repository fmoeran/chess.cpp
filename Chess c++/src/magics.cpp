#include "magics.hpp"

#include <cstdlib>   // rand
#include <time.h>    // srand
#include <bit>       // popcount
#include <format>
#include <iostream>
#include <chrono>
#include <vector>


#define USE_32_BIT_MULTIPLICATIONS


namespace chess
{

	namespace magicGen
	{

		constexpr uint32_t MAX_MAGIC_LOOKUP = 4096;

		// ####################################################
		// GENERATING MAGIC NUMBERS AND LOOKUP TABLES

		Bitmap randomMap() {
			return
				((Bitmap)std::rand() & 0xffULL) |
				(((Bitmap)std::rand() & 0xffULL) << 8) |
				(((Bitmap)std::rand() & 0xffULL) << 16) |
				(((Bitmap)std::rand() & 0xffULL) << 24) |
				(((Bitmap)std::rand() & 0xffULL) << 32) |
				(((Bitmap)std::rand() & 0xffULL) << 40) |
				(((Bitmap)std::rand() & 0xffULL) << 48) |
				(((Bitmap)std::rand() & 0xffULL) << 56);
		}

		Bitmap randomMapFewbits() {
			return randomMap() & randomMap() & randomMap();
		}
		// pseudo moves ignoring other pieceas
		Bitmask rmask(int sq) {
			Bitmask result = 0;
			int rank = sq / 8, file = sq % 8;
			for (int r = rank + 1; r < 7; r++) result |= 1ULL << (r * 8 + file);
			for (int r = rank - 1; r > 0; r--) result |= 1ULL << (r * 8 + file);
			for (int f = file + 1; f < 7; f++) result |= 1ULL << (rank * 8 + f);
			for (int f = file - 1; f > 0; f--) result |= 1ULL << (rank * 8 + f);
			return result;
		}

		Bitmask bmask(int sq) {
			Bitmask result = 0;
			int rank = sq / 8, file = sq % 8;
			for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) result |= 1ULL << (r * 8 + f);
			for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) result |= 1ULL << (r * 8 + f);
			for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) result |= 1ULL << (r * 8 + f);
			for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) result |= 1ULL << (r * 8 + f);
			return result;
		}

		Bitmask ratt(int sq, Bitmap blockMap) {
			Bitmask result = 0;
			int rank = sq / 8, file = sq % 8;

			for (int r = rank + 1; r < 8; r++) {
				Bitmask pos = 1ULL << (r * 8 + file);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int r = rank - 1; r >= 0; r--) {
				Bitmask pos = 1ULL << (r * 8 + file);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int f = file + 1; f < 8; f++) {
				Bitmask pos = 1ULL << (rank * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int f = file - 1; f >= 0; f--) {
				Bitmask pos = 1ULL << (rank * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			return result;
		}

		Bitmask batt(int sq, Bitmap blockMap) {
			Bitmask result = 0;
			int rank = sq / 8, file = sq % 8;

			for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
				Bitmask pos = 1ULL << (r * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
				Bitmask pos = 1ULL << (r * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
				Bitmask pos = 1ULL << (r * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
				Bitmask pos = 1ULL << (r * 8 + f);
				result |= pos;
				if (blockMap & pos) break;
			}
			return result;
		}
		// transforms a board of blockers by a given magic number
		size_t transform(Bitmap board, Bitmask magicNum, int numBits) {
#if defined(USE_32_BIT_MULTIPLICATIONS)
			return
				(unsigned)((int)board * (int)magicNum ^ (int)(board >> 32) * (int)(magicNum >> 32)) >> (32 - numBits);
#else
			return (board * magicNum) >> (64 - numBits);
#endif
		}

		// 
		// \param mask: a view mask from bmask or rmask
		// \index the nth possible
		// \return the ith possible blocker map
		Bitmask mapIndex(Bitmask index, Bitmask mask) {
			// we do this by imagining we map every position of the index num (max 12 bits) to the mask's bits and then return that mask
			Bitmask result = 0;
			Bitmap currentBit;
			while (index) { // we still have 1 or more bits left
				currentBit = mask & (~mask + 1);
				if (index & 1) result |= currentBit;
				mask &= ~currentBit;
				index >>= 1;
			}
			return result;
		}

		bool testMagic(Bitmask magic, int numBits, Bitmask blockers[]) {
			bool used[MAX_MAGIC_LOOKUP] = { false };
			bool failed = false;

			for (int i = 0; i < (1 << numBits); i++) {
				int magicIndex = transform(blockers[i], magic, numBits);
				if (used[magicIndex]) {
					failed = true;
					break;
				}
				else used[magicIndex] = true;
			}
			return !failed;
		}

		Bitmask findMagic(int sq, int numBits, bool isBishop) {
			Bitmask mask = isBishop ? bmask(sq) : rmask(sq);
			Bitmask blockers[MAX_MAGIC_LOOKUP] = { 0ULL };
			Bitmask attacks[MAX_MAGIC_LOOKUP] = { 0ULL };

			for (int i = 0; i < (1 << numBits); i++) {
				blockers[i] = mapIndex(i, mask);
				attacks[i] = isBishop ? batt(sq, blockers[i]) : ratt(sq, blockers[i]);
			}

			for (int i = 0; i < 10000000; i++) {
				Bitmask magic = randomMapFewbits();
				// just quickly check that it has moved some bits to the top, purely for efficiency#
				//if (std::popcount((mask * magic) & 0xff00000000000000) < 3) continue;
				if (testMagic(magic, numBits, blockers)) return magic;
			}
			std::cout << "***FAILED*** " << sq << std::endl;
			return 0ULL;
		}
		const int rBits[64] = {
			12, 11, 11, 11, 11, 11, 11, 12,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			11, 10, 10, 10, 10, 10, 10, 11,
			12, 11, 11, 11, 11, 11, 11, 12
		};

		const int bBits[64] = {
			6, 5, 5, 5, 5, 5, 5, 6,
			5, 5, 5, 5, 5, 5, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 9, 9, 7, 5, 5,
			5, 5, 7, 7, 7, 7, 5, 5,
			5, 5, 5, 5, 5, 5, 5, 5,
			6, 5, 5, 5, 5, 5, 5, 6
		};

		// precomputes all of batt/ratt from a given square
		// places them in the correct index of a lookup table using magic number
		std::vector<Bitmask> getLookup(int sq, Bitmask magic, bool isBishop) {
			Bitmask mask = isBishop ? bmask(sq) : rmask(sq);
			int numBits = isBishop ? bBits[sq] : rBits[sq];
			std::vector<Bitmask> lookupTable(MAX_MAGIC_LOOKUP, 0ULL);
			for (int i = 0; i < (1 << numBits); i++) {
				Bitmask blockerMask = mapIndex(i, mask);
				Bitmask attackMask = isBishop ? batt(sq, blockerMask) : ratt(sq, blockerMask);
				size_t hashIndex = transform(blockerMask, magic, numBits);
				lookupTable[hashIndex] = attackMask;
			}
			return lookupTable;
		}

		void printMagicArrays() {
			using namespace chess;
			//auto t0 = std::chrono::high_resolution_clock::now();

			std::cout << "Bitmask rMagics[64] = {" << std::endl;
			for (int square = 0; square < 64; square++) {
				Bitmap magic = findMagic(square, rBits[square], false);
				std::cout << "0x" << std::format("{:x}", magic) << ',' << std::endl;
			}
			std::cout << "};" << std::endl;
			std::cout << "Bitmask bMagics[64] = {" << std::endl;
			for (int square = 0; square < 64; square++) {
				Bitmap magic = findMagic(square, bBits[square], true);
				std::cout << "0x" << std::format("{:x}", magic) << ',' << std::endl;
			}
			std::cout << "};" << std::endl;

			//auto t1 = std::chrono::high_resolution_clock::now();
			//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
			//std::cout << duration.count() << std::endl;
		}
	}

// ########################################################################
// MAGIC LOOKUPS
// generated from chess::magicGen::printMagicArrays()



	const Bitmask rMagics[64] = {
		0x15c00080400020,
		0x800400008101204,
		0x402001080100820,
		0x21104012100102,
		0x4004000402608,
		0x2010c40a0808082,
		0x8044a080110a,
		0x80560482008081,
		0x80023002088040,
		0x240040880520802,
		0xa28404000004010,
		0x2a018c00008014,
		0x140888404210011,
		0x1802044000008082,
		0x4300218002108002,
		0x2c8000c884008081,
		0x248008400044801,
		0x880420800082820,
		0x1089002000200104,
		0x10842310c011010,
		0x1400805005a00808,
		0x2102008080404,
		0xc1c8180420000202,
		0x104082c0000101,
		0x30800200250441,
		0x204001001010c1a1,
		0x220108a008041041,
		0x402880140840204a,
		0x120960401900806,
		0x138400000100c9,
		0x200a000580108142,
		0x41108200314046,
		0x40202000439802c0,
		0x840005090102812,
		0x680200000001890,
		0x80180000008a10,
		0x8102080000040013,
		0x81002a0300040005,
		0x4080048060800a01,
		0x802280811952d3,
		0x8810040840020030,
		0x8820200090442810,
		0x910060d0010024,
		0x821010020102014,
		0x202a0005820d1062,
		0x2200114000841a,
		0x1403010040010402,
		0x40101020803281,
		0x120124000800a40,
		0xa00020408c1032c0,
		0x4148004048228110,
		0x3010008000068008,
		0x8400202040420801,
		0x230040400092014,
		0x40804020080c1121,
		0x190408010008003,
		0x1445008280002832,
		0x2600c601008013,
		0x100740998001020,
		0x8828200701010010,
		0x88528236824426,
		0x209c01321610100a,
		0xa480122040608c6,
		0x1050208140088047,
	};
	const Bitmask bMagics[64] = {
		0x94808000060408,
		0x2c902d010021007,
		0x1020408390423401,
		0x400900081282a41,
		0x105a00001011040,
		0x2400003208188220,
		0x911b002000010801,
		0x209845000081209,
		0x1905040411002008,
		0x40080600201201c,
		0x92000601204404,
		0xa8211c20050020a,
		0x109008a10001860a,
		0x1222824100402206,
		0x820100008001145,
		0x802910211100021,
		0x202c309248484020,
		0x1000a121298c0010,
		0x202801854209041,
		0xd22a0001408004c,
		0x1042080e05104004,
		0xba10220060802402,
		0x10402000a030104,
		0x80844400220101,
		0x1004411004114191,
		0x3c05082040080205,
		0x241820000021100,
		0x4024100400104104,
		0x412020000004044,
		0x4101090000004208,
		0x4a203008040d208,
		0x44020040020820,
		0xa8100000432010,
		0x450100000008208,
		0x191048002001082,
		0xc00c00440022009,
		0x60408a8008400082,
		0x2000804524011001,
		0x40111800408c084,
		0x5022898c40a0851,
		0xc004200c20290708,
		0x1042044881010a90,
		0x1040a0000401108,
		0x421608120000012,
		0x4100a5000000902,
		0x2090100000a2641,
		0x1040208008481024,
		0x2081c020041104,
		0x412001808040188,
		0x1118040000004042,
		0x4488400101001084,
		0x4092001010504a15,
		0x254020800010860,
		0x10201008c05a025,
		0x2022020000a2001,
		0x60800000380308,
		0x80b4840811198401,
		0x402200010008021,
		0x12a4141058a00002,
		0x42020010010811,
		0x10020200000000b1,
		0x2210010044004013,
		0x242a400200861c8,
		0x22881a000021012,
	};

	MagicLookup::MagicLookup(): square(0), isBishop(false), mask(0), magic(0), numBits(0), lookup(std::vector<Bitmask>(0)) {}

	MagicLookup::MagicLookup(int sq, bool bshp): square(sq), isBishop(bshp) {
		magic = isBishop ? bMagics[sq] : rMagics[sq];
		mask = isBishop ? magicGen::bmask(sq) : magicGen::rmask(sq);
		numBits = isBishop ? magicGen::bBits[sq] : magicGen::rBits[sq];
		lookup = magicGen::getLookup(sq, magic, isBishop);
	}
	Bitmask MagicLookup::operator[](Bitmask allBoard) {
		Bitmask blockers = mask & allBoard;
		size_t ind = magicGen::transform(blockers, magic, numBits);
		return lookup[ind];
	}

}
