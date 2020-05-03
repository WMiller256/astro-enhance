/*
 * coadd.c++
 *
 * William Miller
 * Jul 29, 2019
 *
 * Simplest coadding algorithm - assumes input images are
 * aligned and does not ignore any parts of the image.
 * The stacker routine should be used instead if the images
 * require alignment and the advanced_coadd if there is 
 * foreground to be ignored
 *
 * Usage: coadd <files>
 *
 */
 
#include "enhance.h"		// From personal custom image enhancement library

int main(int argn, char** argv) {
	std::vector<std::string> files;
	for (int ii = 1; ii < argn; ii ++) {
		files.push_back(argv[ii]);
	}
	std::vector<cv::Mat3b> images = read_images(files);
	cv::Mat3b output = coadd(images);
	cv::imwrite("./coadded.tif", output);	
}
