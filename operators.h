/* operators.h
 * 
 * William Miller
 * May 11, 2020
 * 
 * Operators prototypes for dealing with OpenCV classes
 * such as cv::Vec and cv::Mat
 */

#pragma once

#include "enhance.h"

bool operator!=(const cv::Vec3b &l, const cv::Vec3b &r);
bool operator<(const cv::Vec3b &v, const int &i);
bool operator<(const int &i, const cv::Vec3b &v);
std::ostream& operator<<(std::ostream &os, const cv::Vec3b &v);

std::pair<double, double> operator+(const std::pair<double, double> &l, const double &r);
std::pair<double, double> operator+(const double &l, const std::pair<double, double> &r);

std::pair<double, double> operator-(const std::pair<double, double> &l, const std::pair<double, double> &r);
std::pair<double, double> operator+(const std::pair<double, double> &l, const std::pair<double, double> &r);
std::vector<std::pair<double, double>> operator-(const std::vector<std::pair<double, double>> &l, const std::pair<double, double> &r);

// Not technically operators but definitely belong here
double fetch_add(std::atomic<double>* shared, const double &h);
double diff(const std::vector<std::pair<double, double>> &l, const std::vector<std::pair<double, double>> &r);
double distance(const std::pair<double, double> &l, const std::pair<double, double> &r);
