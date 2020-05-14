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

#endif // OPERATORS_H
