/*
 * stacker.c++
 *
 * William Miller
 * Jul 22, 2019
 *
 * Basic deep sky and star stacker which aligns the given images based on
 * the positions of the stars in each image. Images with insufficient 
 * overlap are discarded and foreground is ignored (assuming that it is 
 * darker than the sky). It then coadds the aligned images to produce a 
 * single, higher signal-to-noise, composite image.
 *
 * Possible failure modes - 
 *		 -  Foreground objects are too bright, get detected as stars
 *		 -  Overexposure in the coadded composite from bright clouds
 *
 */
 
#include "enhance.h"		// From personal custom image enhancement library

cv::Mat3b key;
std::string keyframe;
std::vector<cv::Mat3b> aligned;
std::atomic<int> complete;
std::vector<std::string> images;

void align(int start, int end);
 
int main(int argn, char** argv) {
	std::cout << res;
	std::string ofile;
	std::string date = datetime();
	nthreads = max_threads;
	bool advanced;
	double threshold = 0;

	max_features = 10000;
	good_match_percent = 0.01;
	separation_adjustment = 0.05;
	
	po::options_description description("Allowed Options");
	
	try {
		description.add_options()
			("images,i",     po::value<std::vector<std::string> >()->multitoken(), "The images to stack.")
			("output,o",     po::value<std::string>(&ofile)->default_value(date+".coadd.tif"))
			("keyframe,k",   po::value<std::string>(&keyframe))
			("threads,t",    po::value<int>(&nthreads))
			("advanced,a",   po::bool_switch()->default_value(false), "Enable advanced coadding"
				" (where pixels below a certain brightness are ignored).")
			("nfeatures,n",  po::value<int>(&max_features), "The maximum number of matches to create")
			("separation",   po::value<double>(&separation_adjustment), "The maximum permissible separation between"
				" the pixel coordinates of each match in one image compared to another.")
			("matchpercent", po::value<double>(&good_match_percent), "The percentage of matches to keep - matches"
				" are first sorted by match quality and then only this percent of matches are kept.")
			("draw",		 po::bool_switch()->default_value(false), "Draw the matches between each pair of images")
			("threshold",	 po::value<double>(&threshold)->default_value(0.1), "Only in effect if --advanced is true"
				", the brightness below which pixels will be ignored.")
		; 	
	}
	catch (...) {
		std::cout << "Error in boost program options initialization" << std::endl;
		exit(1);
	}
	
	po::variables_map vm;
	po::store(po::command_line_parser(argn, argv).options(description).run(), vm);
	po::notify(vm);
	
	if (nthreads > max_threads) {
		nthreads = max_threads;
	}

	advanced = vm["advanced"].as<bool>();
	draw = vm["draw"].as<bool>();
	if (vm.count("images")) {
		images = vm["images"].as<std::vector<std::string> >();
	}
	else {
		std::cout << "Error - must specify images to stack" << std::endl;
		exit(2);
	}
	if (!vm.count("keyframe")) keyframe = images[0];

	if (nthreads > images.size()) {
		nthreads = images.size();
	}

	std::cout << "{max_features} - " << max_features << std::endl;
	
	std::cout << "Aligning images... " << std::endl;
	key = cv::imread(keyframe);
	int nimages = images.size();
	int block = nimages / nthreads;

	aligned.push_back(key);
	
	complete = 0;
	std::thread threads[nthreads];
	if (block) {
		for (int ii = 0; ii < nthreads - 1; ii ++) {
			threads[ii] = std::thread(align, block * ii, block * (ii + 1));
		}
		threads[nthreads - 1] = std::thread(align, block * (nthreads - 1), nimages);
		
		int percent = -1;
		while (complete != nimages) {
			if (complete * 100 / nimages != percent) {
				percent = complete * 100 / nimages;
				print_percent(complete, nimages);
			}
		}
		
		for (int ii = 0; ii < nthreads; ii ++) {
			threads[ii].join();
		}
	}
	else {
		align(0, images.size());
	}
	
	if (advanced) {
		cv::Mat4b coadded;
		coadded = advanced_coadd(aligned, threshold);
		cv::imwrite(ofile, coadded);	
	}
	else {
		cv::Mat3b coadded;
		coadded = coadd(aligned);
		cv::imwrite(ofile, coadded);	
	}
}

void align(int start, int end) {
	cv::Mat3b image, registered;
	for (int ii = start; ii < end; ii ++) {
		image = cv::imread(images[ii]);
		if (images[ii] != keyframe) {
			align_images(image, key, registered);
			aligned.push_back(registered);
		}
		complete++;
	}	
}
