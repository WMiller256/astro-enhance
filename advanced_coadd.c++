/*
 * coadd.c++
 *
 * William Miller
 * Jul 29, 2019
 *
 * Advanced coadding algorithm which allows setting a lightness
 * threshold below which pixels are ignored and excluded from the average
 *
 * Usage: advanced_coadd <files>
 *
 */
 
#include "enhance.h"		// From personal custom image enhancement library

int main(int argn, char** argv) {
	if (argn == 1) {
		std::cout << "Usage:\nadvanced_coadd <files>\n";
		return(1);
	}
	std::vector<std::string> files;
	for (int ii = 1; ii < argn; ii ++) {
		files.push_back(argv[ii]);
	}
	std::vector<cv::Mat3b> images = read_images(files);
	cv::Mat3b output = advanced_coadd(images, 0.2);
	std::cout << "Writing output..." << std::flush;
	cv::imwrite("./coadded.tif", output);
	std::cout << green+"done"+res+white << std::endl;	
}
