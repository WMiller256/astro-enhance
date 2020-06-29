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
	for (int img = 0; img < images.size(); ++img) {						// Loop through every image in the vector {images}
		images[img].convertTo(color_mat, CV_64FC3);
		cv::Mat3b stars = find_stars(color_mat, max_intensity);
		for (int ii = 0; ii < m.rows; ii ++) {
			for (int jj = 0; jj < m.cols; jj ++) {
				if (brightness(stars.at<cv::Vec3b>(ii, jj)) > brightness(m.at<cv::Vec3b>(ii, jj))) {
					m.at<cv::Vec3b>(ii, jj) = stars.at<cv::Vec3b>(ii, jj);
				}
			}
		}
		print_percent(img, images.size());
	}
	return m;
}

cv::Mat3b find_stars(cv::Mat3b image, uchar max_intensity, 
							int nn, float star_threshold) {				// Find stars in the given image and return a mask of stars
	cv::Mat3b out(image.rows, image.cols, CV_64FC3);					// Initialize output object of type CV_64FC3
	out.setTo(cv::Scalar(0,0,0,0));										// Set every channel to zeros
	for (int ii = 0; ii < out.rows; ii++) {								// Then loop through every pixel
		for (int jj = 0; jj < out.cols; jj++) {	
			cv::Vec3b input = image.at<cv::Vec3b>(ii, jj);						// Find the pixel from the input image and 
			if (brightness(input) > max_intensity*star_threshold && 			// If the brightness is greater than the star threshold
				brightness(input) > brightness(out.at<cv::Vec3b>(ii, jj))) {	// and greater than the current output brightness at that location
				out.at<cv::Vec3b>(ii, jj) = input;								// then apply the new pixel values to the output image.
				for (int kk = -nn; kk < nn; kk ++) {
					for (int mm = -nn; mm < nn; mm ++) {
						if (ii + kk < out.rows && jj + mm < out.cols && ii + kk > 0 && jj + mm > 0) {
							if (brightness(image.at<cv::Vec3b>(ii + kk, jj + mm)) > brightness(out.at<cv::Vec3b>(ii + kk, jj + mm))) {
								out.at<cv::Vec3b>(ii + kk, jj + mm) = image.at<cv::Vec3b>(ii + kk, jj + mm);
							}
						}
					}
				}
			}
		}
	}
	return out;															// Return a mask with stars isolated
}

int brightness(const cv::Vec3b& input) {					    // Find and return the brightness [0-255] of the three 
	return (input.val[0] + input.val[1] + input.val[2])/3;	    // channel pixel {input}
}

bool brighter_than(cv::Vec4b pixel, double threshold) { return ((pixel[0] + pixel[1] + pixel[2]) / 3 > threshold); }

uchar find_max(cv::Mat3b image) {
	uchar max_intensity = 0;
	for (int ii = 0; ii < images[img].rows; ii ++) {		// Loop through each pixel and look for the brightest value
		for (int jj = 0; jj < images[img].cols; jj ++) {
			int value = brightness(images[img].at<cv::Vec3b>(ii, jj));
															// Pull the brightness value for the current pixel
			if (value > max_intensity) {					// And if it is brighter than the previous max
				max_intensity = value;						// Replace the previous max with the new brightness value
			}
		}
	}
	return max_intensity;
}

uchar find_max(std::vector<cv::Mat3b> images) {
	if (images.empty()) return 0;

	std::cout << "Finding maximum intensity..." << std::endl;
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

