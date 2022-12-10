#include "wif.h"
#include <iostream>
#include <fstream>

using namespace corvus;

int main(int argc, char** argv) {
	try {
		// get file to parse
		std::string fname;
		if (2 != argc) {
			std::cout << "usage: " << argv[0] << " [wif to parse]\n";
			return EXIT_FAILURE;
		} else {
			fname = argv[1];
		}

		// make sure file exists
		std::ifstream is(fname);
		if (!is.good()) throw std::invalid_argument("file doesn't exist");

		// do the parsing
		Wif w;
		is >> w;
		std::cout << w;
	} catch (std::exception& e) {
		std::cout << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
