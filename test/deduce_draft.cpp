#include "cell.h"

#include <iostream>
#include <array>
#include <algorithm>

using namespace corvus;

int main() {
	std::array<Cell,4> cells;

	cells[0].mask = {
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
		0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 
		1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 
		1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 
		1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 
	};
	cells[0].warps = 15;
	cells[0].wefts = 30;

	cells[1].mask = {
		1, 1, 0, 0,
		1, 1, 1, 0,
		0, 1, 1, 0,
		0, 1, 1, 1,
		0, 0, 1, 1,
		1, 0, 1, 1,
		1, 0, 0, 1,
		1, 1, 0, 1,
		1, 1, 0, 0,
		1, 1, 1, 0,
		0, 1, 1, 0,
		0, 1, 1, 1,
		0, 0, 1, 1,
		1, 0, 1, 1,
		1, 0, 0, 1,
		1, 1, 0, 1,
	};
	cells[1].warps = 4;
	cells[1].wefts = 16;

	cells[2].mask = {
		1, 0, 0, 1, 0, 1, 1, 1,
		1, 1, 0, 0, 1, 0, 1, 1,
		0, 1, 1, 0, 1, 1, 0, 1,
		0, 0, 1, 1, 1, 1, 1, 0,
	};
	cells[2].warps = 8;
	cells[2].wefts = 4;

	cells[3].mask = {
		0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1,
		0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
		1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
		1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
		0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1,
		1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0,
		1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1,
	};
	cells[3].warps = 16;
	cells[3].wefts = 8;

	for (Cell const& cell : cells) {
		std::vector<uint_fast32_t> threading;
		std::vector< std::vector<uint_fast32_t> > tieup;
		std::vector< std::vector<uint_fast32_t> > treadling;
		uint_fast32_t shafts = cell.layout(threading, tieup, treadling); // currently shafts == treadles


		for (uint_fast32_t i = shafts; i-- > 0; ) {
			for (uint_fast32_t j = 0; j < cell.warps; j++) {
				std::cout << ' ';
				if (threading[j] == i) std::cout << i+1;
				else std::cout << '.';
			}
			std::cout << " |";
			for (uint_fast32_t j = 0; j < tieup.size(); j++) {
				std::cout << ' ';
				if (std::binary_search(tieup[j].cbegin(), tieup[j].cend(), i)) {
					std::cout << 'o';
				} else {
					std::cout << '.';
				}
			}
			std::cout << '\n';
		}
		for (uint_fast32_t j = 0; j < cell.warps; j++) std::cout << "--";
		std::cout << "-+";
		for (uint_fast32_t j = 0; j < shafts; j++) std::cout << "--";
		std::cout << '\n';
		for (uint_fast32_t j = 0; j < cell.wefts; j++) {
			cell.writeWeft(j, std::cout);
			std::cout << " |";
			for (uint_fast32_t i = 0; i < shafts; i++) {
				std::cout << ' ';
				if (std::binary_search(treadling[j].begin(), treadling[j].end(), i)) {
					std::cout << i+1;
				} else {
					std::cout << ' ';
				}
			}
			std::cout << '\n';
		}
		std::cout << '\n';
	}
	return 0;
}

