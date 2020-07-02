
#include "enhance.h"

double star_threshold;
std::string mode;
int nn = 0;

int main(int argn, char** argv) {
	std::vector<std::string> files;

	star_threshold = 0.995;
	for (int ii = 1; ii < argn; ii ++) {
		if (argv[ii][0] != '-') {
			files.push_back(argv[ii]);
		}
		else {
			std::cout << cyan+" ARG: "+res << argv[ii] << " ";
			if (strcmp(argv[ii], "-t") == 0) {
				star_threshold = atof(argv[++ii]);
				std::cout << star_threshold;
			}
			else if (strcmp(argv[ii],"-mode") == 0) {
				mode = argv[++ii];
				std::cout << mode; 
			}
			else if (strcmp(argv[ii], "-nn") == 0) {
				nn = atoi(argv[++ii]);
				std::cout << nn;
			}
			std::cout << std::endl;
		}
	}
	if (mode == "frames") {	
		std::vector<cv::Mat3b> images = read_images(files);
		cv::Mat3b coadded_image = coadd(images);
		cv::Mat3b star_trails = star_trail(images);
		cv::Mat3b output = coadded_image + star_trails;
		imwrite("./composite.tif", output);
	}
	else if (mode == "video") {
		std::vector<cv::Mat3b> frames = extract_frames(files);
		cv::Mat3b coadded_image = coadd(frames);
		cv::Mat3b star_trails = star_trail(frames);
		std::cout << "Coadding... " << std::flush;
		cv::Mat3b output = coadded_image + star_trails;
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

