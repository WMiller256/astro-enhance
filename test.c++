

#include "enhance.h"

int main(int argn, char** argv) {
	std::string _anchor(argv[1]);
	std::string _compar(argv[2]);

	cv::Mat3b anchor = read_image(_anchor);
	cv::Mat3b compar = read_image(_compar);
	cv::Mat3b result;
	align_stars(anchor, compar, result);
}
