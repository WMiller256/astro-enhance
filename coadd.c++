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
 * Usage: 
 * 
 *    coadd [-i|--images] <files> [-h|--scrub-hot-pixels]
 *
 */
 
#include "enhance.h"		// From personal custom image enhancement library

int main(int argn, char** argv) {
	std::vector<std::string> files;
	bool pixel_scrubbing;
	std::string mode;

	po::options_description description("Usage");

	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images to coadd,")
			("scrub-hot-pixels,h", po::bool_switch()->default_value(false), "Scrub the hot pixels out of each image")
			("mode", po::value<std::string>(&mode)->default_value("average"), "The accumulation mode, options are average or median.")
		;
	}
	catch (...) {
		std::cout << "Error in boost program options initialization" << std::endl;
		exit(1);
	}

	po::variables_map vm;
	po::store(po::command_line_parser(argn, argv).options(description).run(), vm);
	po::notify(vm);

	pixel_scrubbing = vm["scrub-hot-pixels"].as<bool>();
	if (vm.count("images")) files = vm["images"].as<std::vector<std::string> >();
	else {
		std::cout << "Error - must specify images to coadd" << std::endl;
		exit(2);
	}

	
	std::vector<cv::Mat> images = read_images(files);
	if (pixel_scrubbing) images = scrub_hot_pixels(images);

	cv::Mat output;
	if (mode == "average") output = coadd(images);
	else if (mode == "median") output = median_coadd(images);
	cv::imwrite("./coadded.tif", output);
}
