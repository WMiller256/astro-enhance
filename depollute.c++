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
    std::vector<std::string> files;

   	po::options_description description("Allowed Options");

	try {
		description.add_options()
			("images,i", po::value<std::vector<std::string> >()->multitoken(), "The images from which to remove light pollution or skyglow,")
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
		std::cout << "Error - must specify input images." << std::endl;
		exit(2);
	}
}
