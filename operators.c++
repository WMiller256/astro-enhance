/* operators.c++
 *
 * William Miller
 * May 11, 2020
 *
 * Custom operators for handling OpenCV objects such 
 * as cv::Vec and cv;:Mat, as well as several for std
 * objects.
 */

#include "operators.h"

bool operator!=(const cv::Vec3b &l, const cv::Vec3b &r) { 
	return (r.val[0] != l.val[0] || r.val[1] != l.val[1] || r.val[2] != l.val[2]); 
}
bool operator<(const cv::Vec3b &v, const int &i) { return (v.val[0] + v.val[1] + v.val[2] < i); }
bool operator<(const int i, const cv::Vec3b &v) { return (v.val[0] + v.val[1] + v.val[2] < i); }
std::ostream& operator<<(std::ostream &os, const cv::Vec3b &v) { 
	os << " (" << std::setfill(' ');
	for (int e = 0; e < 3; e ++) {
		os << std::setw(3) << (int)v.val[e];
		if (e != 2) os << ",";
		else os << ") ";
	}
	return os;
}

// Current standard does not fully support atomic double
double fetch_add(std::atomic<double>* shared, const double &h) {
	double expected = shared->load();
	while (!atomic_compare_exchange_weak(shared, &expected, expected + h))
		;
}

// Not technically operators but definitely belong here
double diff(const std::vector<std::pair<double, double>> &l, const std::vector<std::pair<double, double>> &r) {
// This should be templated but I couldn't get it to work as a templated function so...
	// Protect against size mismatch
	if (l.size() != r.size()) return double(0);
	
	// Static to minimize unnecessary allocations
	static std::atomic<double> result;
	result = 0;

	// Parallelized sum of total separations (very brute force, very intensive).
	// Also note that the range-based for syntax can't be used for outer loop with OMP
#pragma omp parallel for schedule(dynamic)
	for (size_t ii = 0; ii < l.size(); ii ++) {
		double min = distance(l[ii], r[0]);

		// Retrieve the minimum distance
		for (const auto &re : r) min = distance(l[ii], re) < min ? distance(l[ii], re) : min;
		fetch_add(&result, min);
	}
	return result;
}

template <typename T> // This assumes T is a primitive, it will fail otherwise
double distance(const std::pair<T, T> &l, const std::pair<T, T> &r) {
	return (double)std::sqrt(std::pow(std::get<0>(l) - std::get<0>(r), 2) + std::pow(std::get<1>(l) - std::get<1>(r), 2));
}
