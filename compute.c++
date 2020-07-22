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

std::random_device rd;
 
double median(std::vector<double> &v) {
    if (v.empty()) {
        throw "Attempt to take median of empty vector";
    }
    else {
    	const size_t size = v.size();
    	const size_t mid = size / 2;
    	std::nth_element(v.begin(), v.end() + size / 2, v.end());

        return size % 2 ? v[mid] : (v[mid] + *(std::max_element(v.begin(), v.begin() + mid))) / 2.0;
    }
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

std::uniform_int_distribution<> prng(const long min, const long max, std::mt19937 &gen) {
    std::mutex mtx;
    mtx.lock();
    gen = std::mt19937(rd());
    mtx.unlock();
    std::uniform_int_distribution<> distr(min, max);
    return distr;
}
