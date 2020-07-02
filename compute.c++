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

	return size % 2 ? input[size / 2] : (input[size / 2 - 1] + input[size / 2]) / 2;
}

std::vector<double> linspace(const std::pair<double, double> &lim, const size_t &n) {
	double step = (std::get<1>(lim) - std::get<0>(lim)) / static_cast<double>(n - 1);	
	std::vector<double> result(n);
	std::vector<double>::iterator i;
	double e;
	for (i = result.begin(), e = std::get<0>(lim); i != result.end(); ++i, e += step) *i = e;
	
	return result;
}

std::vector<std::pair<double, double>> rotate(const std::vector<std::pair<double, double>> &v, const double &rot) {
	std::vector<std::pair<double, double>> result(v.size());
	for (size_t ii = 0; ii < v.size(); ii ++) {
		result[ii] = std::make_pair(std::get<0>(v[ii]) * std::cos(rot) - std::get<1>(v[ii]) * std::sin(rot),
									std::get<0>(v[ii]) * std::sin(rot) + std::get<1>(v[ii]) * std::cos(rot));
	}
	
	return result;
}
