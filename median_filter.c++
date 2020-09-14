/*
 * median_filter.c++
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
    std::string mode;
    size_t kernel;
    long smoothing;
    long jitter;

   	po::options_description description("Allowed Options");

	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images on which to perform the subtraction.")
			("mode,m", po::value<std::string>(&mode)->default_value("global"), "The filtering mode to apply: global, local, row, or column.")
			("kernel,k", po::value<size_t>(&kernel)->default_value(10), "The kernel to use when filtering, i.e. the size of the region to sample"
			                                                          " for each iteration of the filter. Does not apply to filtering mode 'global'")
			("smoothing,s", po::value<long>(&smoothing)->default_value(0), "The smoothing factor for row and column based filtering. (The size"
			                                                               " of the rolling average to use)")
			("jitter,j", po::value<long>(&jitter)->default_value(0), "Jitter to apply to filter chunking, to prevent co-incident chunk boundaries"
			                                                         " in an image stack.")
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
	FilterMode fmode;
	if (mode == "global") fmode = FilterMode::global;
	else if (mode == "row") fmode = FilterMode::row;
	else if (mode == "col") fmode = FilterMode::col;
	else if (mode == "rowcol") fmode = FilterMode::rowcol;
	else if (mode == "colrow") fmode = FilterMode::colrow;

    std::vector<cv::Mat> images = read_images(files);

    std::cout << "Performing " << mode << " median filtering... " << std::endl;
    std::atomic<int> progress(0);
    #pragma omp parallel for schedule(dynamic)
	for (int ii = 0; ii < images.size(); ii ++) {
        cv::Mat subtracted = median_filter(images[ii], fmode, kernel, smoothing, jitter);

        fs::path path(files[ii]);
        cv::imwrite(path.replace_filename(path.stem().string()+"_msub"+path.extension().string()), subtracted);

        print_percent(progress++, images.size());
	}
}
