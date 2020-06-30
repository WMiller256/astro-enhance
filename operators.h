/* operators.h
 * 
 * William Miller
 * May 11, 2020
 * 
 * Operators prototypes for dealing with OpenCV classes
 * such as cv::Vec and cv::Mat
 */

#ifndef OPERATORS_H
#define OPERATORS_H

#include "enhance.h"

bool operator!=(const cv::Vec3b &l, const cv::Vec3b &r);
bool operator<(const cv::Vec3b &v, const int &i);
bool operator<(const int &i, const cv::Vec3b &v);
std::ostream& operator<<(std::ostream &os, const cv::Vec3b &v);

// Not technically operators but definitely belong here
double fetch_add(std::atomic<double>* shared, const double &h);
double diff(const std::vector<std::pair<double, double>> &l, const std::vector<std::pair<double, double>> &r);

template <typename T> // This assumes T is a primitive, it will fail otherwise
double distance(const std::pair<T, T> &l, const std::pair<T, T> &r);

#endif // OPERATORS_H
