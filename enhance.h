/*
 * enhance.h
 *
 * William Miller
 * May 2, 2020
 *
 * Astrophotography enhancement code
 *
 */

#pragma once

// std
#include <atomic>
#include <cmath>
#include <experimental/filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <valarray>

// Opencv
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

// Boost
#include <boost/program_options.hpp>

class Blob; // Forward declared for use in processing.h

// enhance
#include "iocustom.h"
#include "processing.h"
#include "compute.h"
#include "operators.h"

// Other (some enhance headers depend on these declarations)
#include "blob.h"
#include "chunk.h"
#include "colors.h"
#include "matrix.h"

extern bool draw;
extern int max_features;
extern double good_match_percent;
extern double separation_adjustment;

const int max_threads = std::thread::hardware_concurrency();

const double Pi = 3.14159265358979323846264;

namespace po = boost::program_options;
namespace fs = std::experimental::filesystem;

// File IO
//    Images
cv::Mat3b read_image(std::string file);                                          // Read the single image specified by {file}
std::vector<cv::Mat3b> read_images(std::vector<std::string> files);              // Read the images in the filelist {files}

//     Videos
std::vector<cv::Mat3b> extract_frames(std::vector<std::string> files);           // Extract the frames from a video file into frames in a 
                                                                                 // vector of [Mat3b] objects
std::vector<cv::Mat3b> read_video(std::vector<std::string> videos);              // Extract the frames from a video using FFMPEG library

// Main methods
void subtract(const std::vector<std::string> files, 
              const std::string &_darkframe, const double &factor = 1.0);
cv::Mat4b advanced_coadd(const std::vector<cv::Mat3b> images,                    // Selective coadd ignoring pixels with brightness 
                                 double threshold = 0.3);                        // below {max_intensity}*{threshold}
cv::Mat3b star_trail(const std::vector<cv::Mat3b> images);                       // Find star trails from {images} and return a composite with
                                                                                 // star trails stacked on coadded image

// Star position retrieval etc
cv::Mat3b brightness_find(const cv::Mat3b &image, uchar max_intensisty,          // Finds stars in {image} based on relative brightness, 
                     int nn = 0, double star_threshold = 0.995);                 // returning a [Mat3b] mask of stars 
cv::Mat gaussian_find(const cv::Mat3b &_image, long w, size_t z=12);             // Find stars by 2D Gaussian fitting
Chunk gaussian_estimate(const uchar* pixel, const size_t &cols, const Extent &e);
std::vector<std::pair<double, double>> star_positions(const cv::Mat3b &image, 
                                                      const size_t &n = 100);

// Intensity assessment
uchar find_max(std::vector<cv::Mat3b> images);                                   // Find the brightest pixel in {images}
int brightness(const cv::Vec3b& input);                                          // Find the brightness value of the given 3 channel pixel
bool brighter_than(cv::Vec4b pixel, double threshold);                           // Evaluates if a given pixel is above a brightness threshold
