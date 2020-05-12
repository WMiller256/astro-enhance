/* operators.c++
 *
 * William Miller
 * May 11, 2020
 *
 * Operators for handling OpenCV objects such as cv::Vec 
 * and cv;:Mat.
 */

#include "operators.h"

bool operator!=(const cv::Vec3b &l, const cv::Vec3b &r) { return (r.val[0] == l.val[0] && r.val[1] == l.val[1] && r.val[2] == l.val[2]); }
