/*
 * subtract.c++
 *
 * William Miller
 * Jun 29, 2020
 *
 * Basic image subtraction, mainly for masking off
 * sensor noise and hot pixels by subtracting a 
 * 'dark frame' (i.e. an exposure of same ISO and
 * duration taken at the same ambient temperature
 * around the same time as the primary exposure).
 * 
 * Usage:
 * 
 *    subtract [-i|--images] <files> [-d|--dark-frame] <dark-frame>
 */

#include "enhance.h"

int main(int argn, char** argv) {
	std::vector<std::string> files;
	std::string darkframe;
	double factor;

	po::options_description description("Usage");
	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images to coadd.")
			("dark-frame,d", po::value<std::string>(&darkframe), "The dark frame to subtract off.")
			("factor,f", po::value<double>(&factor), "Multiplication factor for dark frame subtraction.")
		;
	}
	catch (...) {
		std::cout << "Error in boost program options initialization" << std::endl;
		exit(1);
	}

	po::variables_map vm;
	po::store(po::command_line_parser(argn, argv).options(description).run(), vm);
	po::notify(vm);

	if (vm.count("images")) files = vm["images"].as<std::vector<std::string> >();
	else {
		std::cout << "Error - must specify images to subtract from" << std::endl;
		exit(2);
	}

	if (!vm.count("dark-frame")) {
		std::cout << "Error - must specify image to subtract off (i.e. the dark frame)" << std::endl;
		exit(3);
	}

	subtract(files, darkframe, factor);
}
