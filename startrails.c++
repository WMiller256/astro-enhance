
#include "enhance.h"

std::string mode;

int main(int argn, char** argv) {
	std::vector<std::string> files;

   	po::options_description description("Usage");
	try {
		description.add_options()
			("input,i", po::value<std::vector<std::string> >()->multitoken(), "The images on which to perform the operation.")
			("mode,m", po::value<std::string>(&mode)->default_value("frames"), "The processing mode to use, either {frames} if the inputs are image files or {video} if they are video files.")
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
		cv::Mat coadded_image = coadd(images);
		cv::Mat star_trails = star_trail(images);
		cv::Mat output = coadded_image + star_trails;
		imwrite("./composite.tif", output);
	}
	else if (mode == "video") {
		std::vector<cv::Mat> frames = extract_frames(files);
		cv::Mat coadded_image = coadd(frames);
		cv::Mat star_trails = star_trail(frames);
		std::cout << "Coadding... " << std::flush;
		cv::Mat output = coadded_image + star_trails;
		std::cout << green+bright+" done"+res+"." << std::endl;
		output.convertTo(output, CV_8UC3);
		imshow("Image", output);
		imwrite("./composite.tif", output);
	}
	else {
		std::cout << red << "Mode must be specified" << res << std::endl;
	}

	return 0;
}

