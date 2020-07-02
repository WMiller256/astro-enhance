#include "enhance.h"

cv::Mat3b read_image(std::string file) {			// Read the image file specified by {file}
	if (!fs::exists(fs::path(file))) {				// If the file does not exist simply exit with uninitialized return
		std::cout << "Error - file "+file+" does not exist or other file error." << std::endl;
		return cv::Mat3b();	
	}

	cv::Mat3b image = cv::imread(file, cv::IMREAD_COLOR);
	return image;
}

std::vector<cv::Mat3b> read_images(std::vector<std::string> files) {	// Read the image files in the string vector {files}
	std::mutex mtx;														// Create a mutex for pushing images onto {images}
	if (files.empty()) return std::vector<cv::Mat3b>();					// To avoid segmentation fault in case of empty filelist, 
																		// return default-constructed vector of [cv::Mat3b] objects
	std::vector<cv::Mat3b> images;										// Initialize new vector of [cv::Mat3b] objects
	std::cout << "Reading files..." << std::endl;
	int count = 0;
#pragma omp parallel for schedule(dynamic)
	for (int ii = 0; ii < files.size(); ++ii) {							// Then for every file in the list
		cv::Mat3b image;												// create a new temporary [cv::Mat3b] object,
		image = cv::imread(files[ii], cv::IMREAD_COLOR);				// read the {ii}th file from {files} into the temp 
		mtx.lock();
		if (!image.empty()) {											// object. If it is not empty
			images.push_back(image);									// Push it onto the images vector
		}
		else {															// Or if file does not open, print a message and skip 
			std::cout << "Could not open " << yellow << files[ii] << res << " - file may not exist." << std::endl;
		}
		print_percent(count++, files.size());
		mtx.unlock();
	}
	return images;														// Then return the [cv::Mat3b] vector of images
}

cv::Mat3b star_trail(const std::vector<cv::Mat3b> images) {
	if (images.empty()) return cv::Mat3b();								// If the images list is empty, return an empty cv::Mat3b object
	int rows = images[0].rows;
	int cols = images[0].cols;

	cv::Mat3b m(rows, cols, CV_64FC3);									// Create an image initialized to 0's to hold the location of stars
	m.setTo(cv::Scalar(0,0,0,0));
	cv::Mat color_mat;
	uchar max_intensity = find_max(images);

	std::cout << "Finding star trails..." << std::endl;
	for (size_t img = 0; img < images.size(); ++img) {						// Loop through every image in the vector {images}
		images[img].convertTo(color_mat, CV_64FC3);
		cv::Mat3b stars = find_stars(color_mat, max_intensity);

		// Once the stars have been found, combine the star masks
		cv::Vec3b* star = stars.ptr<cv::Vec3b>(0, 0);
		cv::Vec3b* im = m.ptr<cv::Vec3b>(0, 0);
		for (size_t rc = 0; rc < m.rows * m.cols; rc++, star++, im++) {
			if (brightness(*star) > brightness(*im)) {
				*im = *star;
			}
		}
		print_percent(img, images.size());
	}
	return m;
}

cv::Mat3b find_stars(const cv::Mat3b &image, uchar max_intensity, 
							int nn, double star_threshold) {				// Find stars in the given image and return a mask of stars
	cv::Mat3b out = cv::Mat::zeros(image.rows, image.cols, CV_64FC3);		// Initialize output object of type CV_64FC3

	cv::Vec3b* pixel = const_cast<cv::Vec3b*>(image.ptr<cv::Vec3b>(0, 0));
	for (size_t ii = 0; ii < out.rows; ii++) {									// Then loop through every pixel
		for (size_t jj = 0; jj < out.cols; jj++, pixel++) {						// Iterate the data pointer only with the fastest loop
			if (brightness(*pixel) > max_intensity*star_threshold && 			// If the brightness is greater than the star threshold
				brightness(*pixel) > brightness(out.at<cv::Vec3b>(ii, jj))) {	// and greater than the current output brightness at that location
				out.at<cv::Vec3b>(ii, jj) = *pixel;								// then apply the new pixel values to the output image.
				for (int kk = -nn; kk < nn; kk ++) {
					for (int mm = -nn; mm < nn; mm ++) {
						if ( (ii + kk < out.rows && jj + mm < out.cols && ii + kk > 0 && jj + mm > 0) && 
							 (brightness(image.at<cv::Vec3b>(ii + kk, jj + mm)) > brightness(out.at<cv::Vec3b>(ii + kk, jj + mm))) ) {
								out.at<cv::Vec3b>(ii + kk, jj + mm) = image.at<cv::Vec3b>(ii + kk, jj + mm);
						}
					}
				}
			}
		}
	}
	return out;															// Return a mask with stars isolated
}

cv::Mat3b gaussian_find(const cv::Mat3b &_image) {
	// First convert image to grayscale
	cv::Mat image(_image.rows, _image.cols, CV_8U);
	cvtColor(_image, image, cv::COLOR_BGR2GRAY);

	// Retrieve the minimum and total
	double min;
	cv::minMaxLoc(image, &min);

	// Iterating pointer is much faster than using [.at<>()]
	double tot(0);
	uchar* pixel = image.ptr(0, 0);
	for (size_t rc = 0; rc < image.rows * image.cols; rc++, pixel++) tot += *pixel;
	
	std::cout << tot << std::endl;
	return cv::Mat3b();
}

std::vector<std::pair<double, double>> star_positions(const cv::Mat3b &starmask, const size_t &n) {
	std::vector<std::pair<double, double>> positions;
	std::vector<std::vector<cv::Point>> contours;

	// cv::findContours only supports CV_8U images when using cv::RETR_EXTERNAL
	cv::Mat _starmask;
	cvtColor(starmask, _starmask, cv::COLOR_BGR2GRAY);

	// Retrieve the moments of the contours of the star mask. This should produce 
	// a reasonable set of the actual positions of the stars, assuming negligible trailing
	std::vector<cv::Vec4i> heirarchy;
	cv::findContours(_starmask, contours, heirarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

	// Sort by size (might be better to use brightness or do some random sampling but this will suffice for now)
	std::sort(contours.begin(), contours.end(), [](auto &l, auto &r) { return cv::contourArea(l) < cv::contourArea(r); });

	// Extract the centroids positions of the contours
	for (const auto &contour : contours) {
		cv::Moments m = cv::moments(contour, false);
		// If m00 == 0 the contour is illformed
		if (m.m00) positions.push_back(std::make_pair(m.m10 / m.m00, m.m01 / m.m00));
	}
	// Erase all but the first {n} positions
	positions.erase(positions.begin() + n, positions.end());
	return positions;
}

int brightness(const cv::Vec3b& input) {					    // Find and return the brightness [0-255] of the three 
	return (input.val[0] + input.val[1] + input.val[2])/3;	    // channel pixel {input}
}

bool brighter_than(cv::Vec4b pixel, double threshold) { return ((pixel[0] + pixel[1] + pixel[2]) / 3 > threshold); }

uchar find_max(std::vector<cv::Mat3b> images) {
	if (images.empty()) return 0;

	if (images.size() > 1) std::cout << "Finding maximum intensity..." << std::endl;
	uchar max_intensity = 0;
	for (int img = 0; img < images.size(); ++img) {				// Loop through every image in the vector {images}
		print_percent(img, images.size());						// print the percent complete
		for (int ii = 0; ii < images[img].rows; ii ++) {		// Then loop through every pixel in each image and look for stars
			for (int jj = 0; jj < images[img].cols; jj ++) {
				int value = brightness(images[img].at<cv::Vec3b>(ii, jj));
																// Pull the brightness value for the current pixel
				if (value > max_intensity) {					// And if it is brighter than the previous max
					max_intensity = value;						// Replace the previous max with the new brightness value
				}
			}
		}
	}
	return max_intensity;										// Then return the maximum brightness value
}
	
std::vector<cv::Mat3b> extract_frames(std::vector<std::string> files) {	// Extract all frames of given video files to a single
																		// vector of [cv::Mat3b] objects
	if (files.empty()) return std::vector<cv::Mat3b>();					// Guard against seg fault on empty list with return catch
	
	std::vector<cv::Mat3b> frames;										// Initialize a new vector of [cv::Mat3b] objects
	std::cout << "Extracting files..." << std::endl;
	frames = read_video(files);
	return frames;														// Return the [cv::Mat3b] vector {frames} when all files have been
}																		// read in.

std::vector<cv::Mat3b> read_video(std::vector<std::string> videos) {
	/* -    -    -    -    -    -    -    -    - 
	   This works *most* of the time....
	   -    -    -    -    -    -    -    -    - */
	std::vector<cv::Mat3b> frames;

	AVFormatContext* informat_ctx = NULL;
	AVFrame* frame = NULL;
	AVFrame* decframe = NULL;
	AVPixelFormat pixel_format = AV_PIX_FMT_BGR24;
	AVCodecContext* inav_ctx = NULL;
	AVCodec* in_codec = NULL;
	int valid_frame = 0;
	frame = av_frame_alloc();
	decframe = av_frame_alloc();

	avcodec_register_all();
	
	int ret;
	int nframe = 0;

	for (int ii = 0; ii < videos.size(); ii++) {						// For every file in the filelist {videos}, attempt to open
		if ((ret = avformat_open_input(&informat_ctx, videos[ii].c_str(), NULL, NULL)) < 0) {
																					// Check for succesful open, if open failed
			std::cout << red << "Could not open file " << yellow << videos[ii] << res << "." << std::endl;
			return std::vector<cv::Mat3b>();							// Print message and return default constructed
		}																// vector of [cv::Mat3b] objects
		ret = avformat_find_stream_info(informat_ctx, 0);
		if (ret < 0) {													// Then attempt to read in file info, upon failure
			std::cout << red << "Failed to read input file information. " << res << std::endl;
			return std::vector<cv::Mat3b>();							// Print message and return default-constructed vector of [cv::Mat3b]
		}																// objects.

		in_codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);				// Determine the video codec from the file
		if (!in_codec) {
			std::cout << red+white_back+"Error - could not find mpeg4 codec."+res << std::endl;
			exit(1);
		}
		inav_ctx = avcodec_alloc_context3(in_codec);
		inav_ctx->width = 1920;
		inav_ctx->height = 1080;
		inav_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

		avcodec_open2(inav_ctx, in_codec, NULL);						// Open the input codec

		std::vector<uint8_t> framebuf(avpicture_get_size(pixel_format, inav_ctx->width, inav_ctx->height));
		avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), pixel_format, inav_ctx->width, inav_ctx->height);
		while (true) {
			AVPacket pkt;
			av_init_packet(&pkt);
			std::cout << '\r' << cyan << ii << res << "| Frame: " << magenta;
			std::cout << std::setw(4) << std::setfill('0') << nframe++ << res;
			ret = av_read_frame(informat_ctx, &pkt);					// Read the next frame in
			avcodec_decode_video2(inav_ctx, frame, &valid_frame, &pkt);	// Decode the next frame
			if (ret == 0 && valid_frame == 0) {									// If a valid frame was decoded
				struct SwsContext *img_convert_ctx;						// Convert to OpenCV compatible format
				img_convert_ctx = sws_getContext(
					inav_ctx->width,
					inav_ctx->height,
					inav_ctx->pix_fmt,
					inav_ctx->width,
					inav_ctx->height,
					AV_PIX_FMT_BGR24,
					SWS_BICUBIC,
					NULL,
					NULL,
					NULL);
				sws_scale(img_convert_ctx,
					decframe->data,
					decframe->linesize,
					0,
					decframe->height,
					frame->data,
					frame->linesize);
				sws_freeContext(img_convert_ctx);
																		// Then create a new [cv::Mat] object
				cv::Mat _frame(frame->height, frame->width, CV_64FC3, framebuf.data(), frame->linesize[0]);
				frames.push_back(_frame);									// And add it to the output vector
			}
			else {
				break;													// Once an invalid frame is read, break out of loop
			}
		}
		std::cout << std::endl;
	}
	std::cout << frames[0].rows << " " << frames[0].cols << std::endl;
	return frames;														// And return 
}

