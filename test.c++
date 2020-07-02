

#include "enhance.h"

int main(int argn, char** argv) {
	std::string _anchor(argv[1]);
//	std::string _compar(argv[2]);

	cv::Mat3b anchor = read_image(_anchor);
//	cv::Mat3b compar = read_image(_compar);
	cv::Mat result;
//	align_stars(anchor, compar, result);
	result = gaussian_find(anchor, 10, 12);
	cv::imwrite("starmask.tif", result);
}
