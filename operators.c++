/* operators.c++
 *
 * William Miller
 * May 11, 2020
 *
 * Operators for handling OpenCV objects such as cv::Vec 
 * and cv;:Mat.
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

