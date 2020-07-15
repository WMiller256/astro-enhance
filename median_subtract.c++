/*
 * median_subtract.c++
 *
 * William Miller
 * Jul 15, 2020
 *
 * Simple routine which calculates a global median for each 
 * color channel and subtracts it off from the entire image.
 * Useful in normalizing deep-sky exposures with different 
 * brightnesses.
 *
 */

#include "enhance.h"

int main(int argn, char** argv) {
    std::cout << res;
    std::vector<std::string> files;

   	po::options_description description("Allowed Options");

	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images on which to perform the subtraction.")
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

    std::vector<cv::Mat3b> images = read_images(files);

    std::cout << "Performing per-channel global median subtraction... " << std::endl;
    std::atomic<int> progress(0);
    #pragma omp parallel for schedule(dynamic)
	for (int ii = 0; ii < images.size(); ii ++) {
        cv::Mat3b subtracted = median_subtract(images[ii]);

        fs::path path(files[ii]);
        cv::imwrite(path.replace_filename(path.stem().string()+"_msub"+path.extension().string()), subtracted);

        print_percent(progress++, images.size());
	}
}
