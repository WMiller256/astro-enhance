/*
 * gifrepair.c++
 *
 * William Miller
 * Dec 16, 2021
 *
 * Basic programmatic GIF artifact repair using
 * Gaussian blurring, selective Gaussian blurring, 
 * and unsharp masking.
 */

#include "enhance.h"

int main(int argn, char** argv) {
    std::cout << res;
    std::vector<std::string> files;
    double blur;
    double scale;

   	po::options_description description("Usage");

	try {
		description.add_options()
			("input,i", po::value<std::vector<std::string> >()->multitoken()->required(), "The input files on which to perform the subtraction (required, must be of type GIF).")
			("blur,b", po::value<double>(&blur)->default_value(1000.0), "The strength of the blur effect. HIGHER values create LESS blur! (optional)")
			("scale,s", po::value<double>(&scale)->default_value(1.0), "The scaling to apply to input files")
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
		std::cout << "Error - must specify input files." << std::endl;
		exit(2);
	}
	
    for (const auto &file : files) { 
        // Read in and process the frames from {file}
        std::vector<cv::Mat> frames;
        cv::VideoCapture f(file);
        double fps = f.get(cv::CAP_PROP_FPS);
        int ex = static_cast<int>(f.get(cv::CAP_PROP_FOURCC));
                
        cv::Size size;
        while (true) {
            cv::Mat in, resized, blurred, out;
            f >> in;
            if (in.empty()) break;

            size = cv::Size(in.cols, in.rows) * scale;
            
            cv::resize(in, resized, cv::Size(), 3.0, 3.0, cv::INTER_CUBIC);           // Scale up
            cv::GaussianBlur(resized, blurred, cv::Size(0, 0), resized.cols / blur);  // Apply Gaussian blur
            cv::GaussianBlur(blurred, blurred, cv::Size(0, 0), 2.0);                  // Apply median blur
            cv::resize(blurred, out, size, cv::INTER_CUBIC);                          // Scale back to original size (modified by {scale})

            frames.push_back(out);
        }
        f.release();

        // Write out the processed frames
        cv::VideoWriter output;
        fs::path path(file);
        path.replace_extension(".mp4");
        output.open(path.string(), cv::VideoWriter::fourcc('h', 'e', 'v', '1'), fps, size);
        for (const auto &frame : frames) {
            output << frame;
        }
    }
    return 0;
}
