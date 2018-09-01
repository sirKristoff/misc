/*
 * main.cpp
 *
 *  Created on: 01.09.2018
 *      Author: Krzysztof Lasota
 */

#include <iostream>

#include "histogram.hpp"

int main(int argc, char** argv)
{
	unsigned n = ((1<argc)?atoi(argv[1])-1:0);
	double seed = 1;
	Histogram h(1,seed);

	for (unsigned i = 0; i < n; ++i) {
		h += h << 1;
	}

	std::cout << h << std::endl;

	return 0;
}
