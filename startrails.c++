
#include "enhance.h"


int main(int argn, char** argv) {
	std::vector<std::string> files;
    std::string mode;
    uint threshold; 

   	po::options_description description("Usage");
	try {
		description.add_options()
			("input,i", po::value<std::vector<std::string> >()->multitoken(), "The images on which to perform the operation.")
			("mode,m", po::value<std::string>(&mode)->default_value("frames"), "The processing mode to use, either {frames} if the "
			                                                                   "inputs are image files or {video} if they are video "
			                                                                   "files. Optional.")
			("threshold,t", po::value<uint>(&threshold)->default_value(0), "The brightness threshold below which pixels will use the " 
			                                                               "mean value instead of lightest value during accumulation "
			                                                               "[0-255]. Optional.")
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

	if (vm.count("input")) files = vm["input"].as<std::vector<std::string> >();
	else {
        std::cout << description << std::endl;
   		exit(2);
	}

	if (mode == "frames") {	
		std::vector<cv::Mat> images = read_images(files);
		cv::Mat star_trails = star_trail(images, threshold);
		imwrite("./composite.tif", star_trails);
	}
	else if (mode == "video") {
		std::vector<cv::Mat> frames = extract_frames(files);
		cv::Mat star_trails = star_trail(frames, threshold);
		std::cout << green+bright+" done"+res+"." << std::endl;
		star_trails.convertTo(star_trails, CV_8UC3);
		imshow("Image", star_trails);
		imwrite("./composite.tif", star_trails);
	}
	else {
		std::cout << red << "Mode must be specified" << res << std::endl;
	}

	return 0;
}

