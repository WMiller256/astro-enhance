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

#endif // OPERATORS_H
