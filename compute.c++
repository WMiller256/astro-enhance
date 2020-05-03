/*
 * compute.c++
 * 
 * William Miller
 * Aug 2, 2019
 *
 * Basic computation functions for image enhancement library
 *
 */

#include "compute.h"
 
double median(std::vector<double> input) {
	std::sort(input.begin(), input.end());
	size_t size = input.size();
	if (size % 2 == 0) {
		return (input[size / 2 - 1] + input[size / 2]) / 2;
	}
	else {
		return input[size / 2];
	}
}

double median(std::vector<float> input) {
	std::sort(input.begin(), input.end());
	size_t size = input.size();
	if (size % 2 == 0) {
		return (input[size / 2 - 1] + input[size / 2]) / 2;
	}
	else {
		return input[size / 2];
	}
}
