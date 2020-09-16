/*
 * processing.h
 * 
 * William Miller
 * Aug 1, 2019
 *
 *
 */

#pragma once

#include "enhance.h"
#include "chunk.h"

enum class FilterMode {global, row, col, rowcol, colrow};

void accumulate(const std::vector<cv::Mat> images, cv::Mat &m, const size_t &n);
void subtract(const std::vector<std::string> files, 
              const std::string &_darkframe, const double &factor = 1.0);

cv::Mat coadd(const std::vector<cv::Mat> &images);
cv::Mat median_coadd(const std::vector<cv::Mat> &images);
void _median_coadd(const int tidx, const std::vector<cv::Mat> &images, std::vector<cv::Mat> &output, size_t offset, const size_t end);

std::vector<cv::Mat> scrub_hot_pixels(const std::vector<cv::Mat> images);

void align_stars(cv::Mat &anc, cv::Mat &com, cv::Mat &result);
void align_images(cv::Mat &im1, cv::Mat &im2, cv::Mat &imreg);				// Aligns two given images and stores aligned result in {imreg}

// Calculates the separation (pixel distance) between two matched features
double find_separation(cv::DMatch m, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);
// Calculates the separation (pixel distance) between all matched features in {matches}
std::vector<double> find_separations(std::vector<cv::DMatch> matches, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);

void interpolate_simple(cv::Mat &im, const cv::Mat &starmask);
std::vector<Blob> blob_extract(const cv::Mat &im);
void _blob_extract(const cv::Mat &mask, Blob &blob, uchar* pixel, uchar* start);

cv::Mat median_filter(const cv::Mat &image, const FilterMode mode = FilterMode::global, const bool norm = false, 
                      const bool stretch = false, const size_t kernel = 10, const long smoothing = 0, const long jitter = 0);
void normalize(cv::Mat &image, const double &c);
void smooth_median(std::vector<Chunk> &chunks, const long smoothing);
