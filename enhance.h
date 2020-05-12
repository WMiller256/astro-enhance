#ifndef ENHANCE_H
#define ENHANCE_H

#include <colors.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp> 

#include <boost/program_options.hpp>

#include "iocustom.h"
#include "processing.h"
#include "compute.h"
#include "operators.h"

extern bool draw;
extern int max_features;
extern float good_match_percent;
extern float separation_adjustment;
	
namespace po = boost::program_options;

std::vector<cv::Mat3b> read_images(std::vector<std::string>			// Read the images in the filelist {files}
							   files);
cv::Mat3b coadd(const std::vector<cv::Mat3b> images);					// Average all pixels from {images}
cv::Mat3b advanced_coadd(const std::vector<cv::Mat3b> images,		// Selective coadd ignoring pixels with brightness 
								 float threshold = 0.3);						// below {max_intensity}*{threshold}
cv::Mat3b star_trail(const std::vector<cv::Mat3b> images);			// Find star trails from {images} and return a composite with
																						// star trails stacked on coadded image
cv::Mat3b find_stars(cv::Mat3b image, uchar max_intensisty,			// Finds stars in {image} based on relative brightness, 
							int nn = 0, float star_threshold = 0.995);	// returning a [Mat3b] mask of stars 
uchar find_max(std::vector<cv::Mat3b> images);							// Find the brightest pixel in {images}
int brightness(const cv::Vec3b& input);									// Find the brightness value of the given 3 channel pixel
bool brighter_than(cv::Vec4b pixel, double threshold);				// Evaluates if a given pixel is above a brightness threshold

std::vector<cv::Mat3b> extract_frames(std::vector<std::string>		// Extract the frames from a video file into frames in a 
								  files);											// vector of [Mat3b] objects
std::vector<cv::Mat3b> read_video(std::vector<std::string>			// Extract the frames from a video using FFMPEG library
								  videos);						
#endif
