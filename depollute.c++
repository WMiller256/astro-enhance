/*
 * depollute.c++
 *
 * William Miller
 * Jul 13, 2020
 *
 * Routine to remove light pollution from light frames.
 * Also useful in removing skyglow from frames taken in 
 * Moonlight or near twilight. 
 *
 */

#include "enhance.h"

int main(int argn, char** argv) {
    std::cout << res;
    std::vector<std::string> files;
    std::string _mode;
    findBy mode;
    size_t scale;
    size_t z;

   	po::options_description description("Allowed Options");

	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images from which to remove light pollution or skyglow.,")
			("scale,s", po::value<size_t>(&scale)->default_value(10), "The spatial scale of the depollution model elements.")
			("z-score,z", po::value<size_t>(&z)->default_value(8), "The z-score threshold for star detection.")
			("detection,d", po::value<std::string>(&_mode)->default_value("gaussian"), "The star detection mode, options are \"gaussian\" or \"brightness\"")
		;
	}
	catch (...) {
		std::cout << "Error in boost program options initialization" << std::endl;
		exit(0);
	}

  	po::variables_map vm;
    try {
    	po::store(po::command_line_parser(argn, argv).options(description).run(), vm);
    	po::notify(vm);
    }
    catch (...) {
        std::cout << description << std::endl;
        exit(1);
    }

	if (vm.count("images")) files = vm["images"].as<std::vector<std::string> >();
	else {
		std::cout << "Error - must specify input images." << std::endl;
		exit(2);
	}
    if (_mode == "gaussian") mode = findBy::gaussian;
    else if (_mode == "brightness") mode = findBy::brightness;

    std::vector<cv::Mat3b> images = read_images(files);
	for (auto &image : images) {
        cv::Mat model = depollute(image, scale, z, mode);
        cv::imwrite("model.tif", model);
        cv::imwrite("depolluted.tif", image);
	}
}
