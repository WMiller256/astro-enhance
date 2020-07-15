

#include "enhance.h"

int main(int argn, char** argv) {
	std::string _image(argv[1]);
	cv::Mat image = read_image(_image);

	cv::imwrite("result.tif", median_subtract(image));
}
