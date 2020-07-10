/*
 * processing.h
 * 
 * William Miller
 * Aug 1, 2019
 *
 *
 */

#pragma once

#include <atomic>

#include "enhance.h"

void accumulate(const std::vector<cv::Mat> images, cv::Mat &m, const size_t &n);
cv::Mat3b coadd(const std::vector<cv::Mat3b> &images);
cv::Mat3b median(const std::vector<cv::Mat3b> &images);
void _median(const int tidx, const std::vector<cv::Mat3b> &images, std::vector<cv::Mat> &output, size_t offset, const size_t end);

std::vector<cv::Mat3b> scrub_hot_pixels(const std::vector<cv::Mat3b> images);

void align_stars(cv::Mat &anc, cv::Mat &com, cv::Mat &result);
void align_images(cv::Mat &im1, cv::Mat &im2, cv::Mat &imreg);				// Aligns two given images and stores aligned result in {imreg}

// Calculates the separation (pixel distance) between two matched features
double find_separation(cv::DMatch m, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);
// Calculates the separation (pixel distance) between all matched features in {matches}
std::vector<double> find_separations(std::vector<cv::DMatch> matches, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);

void interpolate_simple(cv::Mat3b &im, const cv::Mat &starmask);
std::vector<Blob> blob_extract(const cv::Mat &im);
void _blob_extract(const cv::Mat &mask, Blob &blob, uchar* pixel, uchar* start);
