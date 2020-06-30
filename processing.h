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

void accumulate(const std::vector<cv::Mat> images, cv::Mat &m, const size_t &n);
cv::Mat3b coadd(const std::vector<cv::Mat3b> images);
std::vector<cv::Mat3b> scrub_hot_pixels(const std::vector<cv::Mat3b> images);
cv::Mat4b advanced_coadd(const std::vector<cv::Mat3b> images, double threshold);

void align_stars(cv::Mat &anc, cv::Mat &com, cv::Mat &result, bool translation = true, 
				 bool rotation = false, bool perspective = false);
void align_images(cv::Mat &im1, cv::Mat &im2, cv::Mat &imreg);				// Aligns two given images and stores aligned result in {imreg}

// Calculates the separation (pixel distance) between two matched features
float find_separation(cv::DMatch m, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);
// Calculates the separation (pixel distance) between all matched features in {matches}
std::vector<float> find_separations(std::vector<cv::DMatch> matches, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2);
