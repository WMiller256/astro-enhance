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
#include <random>
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

// Eigen
#include <Eigen/Dense>


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

const int max_threads = std::thread::hardware_concurrency() - 1;

const double Pi = 3.14159265358979323846264;

namespace po = boost::program_options;
namespace fs = std::experimental::filesystem;

enum class FindBy {gaussian, brightness};

// TODO refactor to improve readin scheme - read images only as they 
// are needed instead of reading all input images in at the beginning 
// and then performing the operations

// File IO
//    Images
cv::Mat read_image(std::string file);                                          // Read the single image specified by {file}
std::vector<cv::Mat> read_images(std::vector<std::string> files);              // Read the images in the filelist {files}

//     Videos
std::vector<cv::Mat> extract_frames(std::vector<std::string> files);           // Extract the frames from a video file into frames in a 
                                                                                 // vector of [Mat] objects
std::vector<cv::Mat> read_video(std::vector<std::string> videos);              // Extract the frames from a video using FFMPEG library

// Main methods
cv::Mat4b advanced_coadd(const std::vector<cv::Mat> images,                    // Selective coadd ignoring pixels with brightness 
                                 double threshold = 0.3);                        // below {max_intensity}*{threshold}
cv::Mat star_trail(const std::vector<cv::Mat> images);                       // Find star trails from {images} and return a composite with
cv::Mat depollute(cv::Mat &images, const size_t size = 50, 
                  const size_t z=8, const FindBy find = FindBy::gaussian);
                                                                                 // star trails stacked on coadded image

// Star position retrieval etc
cv::Mat brightness_find(const cv::Mat &_image, const size_t z=8);
cv::Mat brightness_find_legacy(const cv::Mat &image, uchar max_intensisty,   // Finds stars in {image} based on relative brightness, 
                     int nn = 0, double star_threshold = 0.995);                 // returning a [Mat] mask of stars |LEGACY|
cv::Mat gaussian_find(const cv::Mat &_image, long w, size_t z=8);              // Find stars by 2D Gaussian fitting
Chunk gaussian_estimate(const uchar* pixel, const size_t &cols, const Extent &e);
std::vector<std::pair<double, double>> star_positions(const cv::Mat &image, 
                                                      const size_t &n = 100);

Eigen::MatrixXd depollute_region(cv::Mat &image, const cv::Mat &stars, const long &r, 
                                 const long &c, const int &b, const size_t &size);

// Intensity assessment
uchar find_max(std::vector<cv::Mat> images);                                   // Find the brightest pixel in {images}
int brightness(const cv::Vec3b& input);                                          // Find the brightness value of the given 3 channel pixel
bool brighter_than(cv::Vec4b pixel, double threshold);                           // Evaluates if a given pixel is above a brightness threshold
