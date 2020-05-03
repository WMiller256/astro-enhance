/*
 * processing.c++
 * 
 * William Miller
 * Aug 1, 2019
 *
 * Image processing and manipulation functions for image
 * enhancement. Primarily geared toward stellar photography
 * and astrophotography. Included are functions to selectively
 * coadd, primitively coadd, and align image stacks.
 *
 */

#include "processing.h"

int max_features;
bool draw;
float good_match_percent;
float separation_adjustment;

cv::Mat3b coadd(const std::vector<cv::Mat3b> images) {
	std::cout << "{max_features} - " << max_features << std::endl;
	std::cout << "{good_match_percent} - " << good_match_percent << std::endl;
	std::cout << "{separation_adjustment} - " << separation_adjustment << std::endl;
   if (images.empty()) return cv::Mat3b();

	// Create a 0 initialized image to use as accumulator
	cv::Mat m(images[0].rows, images[0].cols, CV_64FC3);
	m.setTo(cv::Scalar(0,0,0,0));

	// Use a temp image to hold the conversion of each input image to CV_64FC3
	// This will be allocated just the first time, since all the images have
	// the same size.
	cv::Mat temp;
	std::cout << "Converting the input images to CV_64FC3 ..." << std::endl;
	for (int ii = 0; ii < images.size(); ii ++) {
		print_percent(ii, images.size());
		images[ii].convertTo(temp, CV_64FC3);
		
		m += temp;
	}

	std::cout << "Dividing... " << std::flush;
	// Convert back to CV_8UC3 type, applying the division to get the actual mean
	m.convertTo(m, CV_8U, 1. / images.size());
	std::cout << green+"done"+res+white+"." << std::endl;
	return m;
}

cv::Mat4b advanced_coadd(const std::vector<cv::Mat3b> images, double threshold) {
// Broken 
   if (images.empty()) return cv::Mat();
   
   // Create a vector of matricies with 4 vector elements per matrix element
   std::vector<cv::Mat4b> modified;

	// Create a 0 initialized image to use as accumulator
	cv::Mat m(images[0].rows, images[0].cols, CV_32FC4);
	m.setTo(cv::Scalar(0, 0, 0, 255));
	
	// Create a mask to hold the weights for each pixel
	int** mask = new int*[m.rows];
	for (int ii = 0; ii < m.rows; ii ++) {
		mask[ii] = new int[m.cols];
		for (int jj = 0; jj < m.cols; jj ++) {
			mask[ii][jj] = 1;
		}
	}

	uchar max = find_max(images);
	std::cout << "Maximum is " << (int)max << std::endl;
	std::cout << "Performing selective coadding ..." << std::endl;
	for (int ii = 0; ii < images.size(); ii++) {
		long transparent = 0;
		print_percent(ii, images.size());
		cv::Mat current(images[ii].rows, images[ii].cols, CV_32FC4);
		cv::cvtColor(images[ii], current, cv::COLOR_BGR2BGRA);
		std::cout << type2str(current.type()) << std::endl;
		for (int jj = 0; jj < current.rows; jj ++) {
			for (int kk = 0; kk < current.cols; kk ++) {
				cv::Vec4b& pixel = current.at<cv::Vec4b>(jj, kk);
				if (!brighter_than(pixel, max*threshold)) {
					pixel[3] = 0.0;
					transparent ++;
				}
			}
		}
		current.convertTo(current, CV_32FC4);
		modified.push_back(current);
		if (transparent) {
//			std::cout << transparent << " pixels below threshold." << std::endl;
		}
		else {
//			std::cout << "No pixels detected below threshold" << std::endl;
		}
	}
	std::cout << "Accumulating..." << std::endl; 
	for (int ii = 0; ii < modified.size(); ii ++) {
		modified[ii].convertTo(modified[ii], CV_32FC4);
		std::cout << type2str(modified[ii].type()) << std::endl;
		for (int jj = 0; jj < m.rows; jj ++) {
			for (int kk = 0; kk < m.cols; kk ++) {
				cv::Vec4b pixel = modified[ii].at<cv::Vec4b>(jj, kk);
				cv::Vec4b& mpixel = m.at<cv::Vec4b>(jj, kk);
				if (pixel[3] != 0.0) {
					mpixel[0] += pixel[0];
					mpixel[1] += pixel[1];
					mpixel[2] += pixel[2];
					mask[jj][kk] += 1;
				}				
/*				if (jj == m.rows/2 && kk == m.cols/2) {
					std::cout << "BGRA mpixel: (" << (float)mpixel[0] << ", " << (float)mpixel[1] << ", " << (float)mpixel[2] << ", " << (float)mpixel[3] << ")" << std::endl;
					std::cout << "BGRA pixel:  (" << (float)pixel[0] << ", " << (float)pixel[1] << ", " << (float)pixel[2] << ", " << (float)pixel[3] << ")" << std::endl;
					mpixel = m.at<cv::Vec4b>(jj, kk);
					std::cout << "BGRA mpixel: (" << (float)mpixel[0] << ", " << (float)mpixel[1] << ", " << (float)mpixel[2] << ", " << (float)mpixel[3] << ")\n" << std::endl;
				}
*/			}
		}
	}
	cv::imwrite("temp.png", m);
	std::cout << "Dividing... " << std::endl;
	for (int ii = 0; ii < m.rows; ii ++) {
		print_percent(ii, m.rows);
		for (int jj = 0; jj < m.cols; jj ++) {
			m.at<cv::Vec4b>(ii, jj) = m.at<cv::Vec4b>(ii, jj) / (float)mask[ii][jj];
		}
	}
	
	std::cout << "Converting... " << std::flush;
	cv::Mat4b output(m.rows, m.cols, CV_32FC4);
//	cv::cvtColor(m, output, cv::COLOR_BGRA2BGR);
	m.convertTo(output, CV_8U, 1. / modified.size());
	std::cout << green+"done."+res+white << std::endl;
	return output;
}

void align_images(cv::Mat &im1, cv::Mat &im2, cv::Mat &im1Reg) {
	
	// Convert images to grayscale
	cv::Mat im1Gray, im2Gray, h;
	cvtColor(im1, im1Gray, cv::COLOR_BGR2GRAY);
	cvtColor(im2, im2Gray, cv::COLOR_BGR2GRAY);
	
	// Variables to store keypoints and descriptors
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	cv::Mat descriptors1, descriptors2;
	
	// Detect ORB features and compute descriptors.
	cv::Ptr<cv::Feature2D> orb = cv::ORB::create(max_features);
	orb->detectAndCompute(im1Gray, cv::Mat(), keypoints1, descriptors1);
	orb->detectAndCompute(im2Gray, cv::Mat(), keypoints2, descriptors2);
	
	// Match features.
	std::vector<cv::DMatch> matches;
	cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
	matcher->match(descriptors1, descriptors2, matches, cv::Mat());
	
	std::vector<float> separations = find_separations(matches, keypoints1, keypoints2);
	
	// Sort matches by score
	std::vector<std::pair<float, cv::DMatch> > paired;
	for (int ii = 0; ii < matches.size(); ii ++) {
		std::pair<float, cv::DMatch> p(separations[ii], matches[ii]);
		paired.push_back(p);
	}
	std::sort(paired.begin(), paired.end());
	
	for (int ii = 0; ii < paired.size(); ii ++) {
  		separations[ii] = paired[ii].first;
  		matches[ii] = paired[ii].second;	
	}
		
	std::sort(matches.begin(), matches.end());
	const int numGoodMatches = (matches.size() * good_match_percent > 3 ? matches.size() * good_match_percent : 4);
	matches.erase(matches.begin()+numGoodMatches, matches.end());   
	std::cout << "{matches.size()} - " << matches.size() << std::endl;
	
	// Draw best matches
	if (draw) {
		cv::Mat imMatches;
		cv::drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
		cv::imwrite("matches.jpg", imMatches);
   }
		
	// Extract location of good matches
	std::vector<cv::Point2f> points1, points2;
	
	for (int ii = 0; ii < matches.size(); ii ++) {
		points1.push_back(keypoints1[matches[ii].queryIdx].pt);
		points2.push_back(keypoints2[matches[ii].trainIdx].pt);
	}
	
	// Find homography
	h = cv::findHomography(points1, points2, cv::RANSAC);
	
	// Use homography to warp image
	std::cout << "(" << h.size().width << "x" << h.size().height << ")" << std::endl;
	cv::warpPerspective(im1, im1Reg, h, im2.size());
	
}
 
// Calculates the separation (pixel distance) between two matched features
float find_separation(cv::DMatch m, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2) {
	return sqrt(pow(kp1[m.queryIdx].pt.x - kp2[m.trainIdx].pt.x, 2) + pow(kp1[m.queryIdx].pt.y - kp2[m.trainIdx].pt.y, 2));
}

std::vector<float> find_separations(std::vector<cv::DMatch> matches, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2) {
	std::vector<float> separations;
	for (int ii = 0; ii < matches.size(); ii ++) {
		separations.push_back(find_separation(matches[ii], kp1, kp2));
	}
	return separations;
}
