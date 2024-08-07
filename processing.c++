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
double good_match_percent;
double separation_adjustment;
std::atomic<size_t> progress(0);

void accumulate(const std::vector<cv::Mat> images, cv::Mat &m, const size_t &n) {
    // Use a temp image to hold the conversion of each input image to CV_64FC3
    // This will be allocated just the first time, since all the images have
    // the same size.
    cv::Mat temp(m.rows, m.cols, CV_64FC3);
    for (auto im : images) {
        im.convertTo(temp, CV_64FC3);
        m += temp;
        print_percent(progress++, n);
    }
}

void subtract(const std::vector<std::string> files, const std::string &_darkframe, const double &factor) {
    size_t idx = 0;
    fs::path path;
    std::vector<cv::Mat> images = read_images(files);
    cv::Mat darkframe = read_image(_darkframe);
    cv::Mat out;
    for (const auto &im : images) {
        path = fs::path(files[idx++]);
        out = im - darkframe * factor;
        cv::imwrite(path.replace_filename(path.stem().string()+"_sub"+path.extension().string()), out);
        print_percent(idx, files.size());
    }
}

cv::Mat coadd(const std::vector<cv::Mat> &images) {
//    std::cout << "{max_features} - " << max_features << std::endl;
//    std::cout << "{good_match_percent} - " << good_match_percent << std::endl;
//    std::cout << "{separation_adjustment} - " << separation_adjustment << std::endl;
    if (images.empty()) return cv::Mat();

    // Protect against hardware concurrency larger than number of images
    const int nthreads = max_threads > images.size() ? images.size() : max_threads;

    // Create a 0 initialized image to use as accumulator for each thread
    std::valarray<cv::Mat> m(cv::Mat(images[0].rows, images[0].cols, CV_64FC3), nthreads);
    for (auto &e : m) {
        e.setTo(cv::Scalar(0, 0, 0, 0));
    }

    std::cout << "Accumulating..." << std::endl;
    // Create either {nthreads} threads and give each a subset of images to process
    // or create one thread for each image. The latter only when {nthreads} < {images.size()}
    const size_t size = images.size();
    const int block = size > nthreads ? size / nthreads : 1;
    const int nt = block > 0 ? nthreads : size;
    std::vector<std::thread> threads(nt);
    std::vector<cv::Mat> v(nt);

    progress = 0;   // Have to reset progress to zero because the same object is used for all progress tracking
    print_percent(progress, size);
    accumulate(images, m[0], size);
/*    for (int ii = 0; ii < nt - 1; ii ++) {
        threads[ii] = std::thread(accumulate, std::vector<cv::Mat>(images.begin() + ii * block, images.begin() + (ii + 1) * block), 
                                  std::ref(m[ii]), std::ref(size));
    }
    threads.back() = std::thread(accumulate, std::vector<cv::Mat>(images.begin() + block * (nt - 1), images.end()),
                                 std::ref(m[nt-1]), std::ref(size));
    for (auto &t : threads) t.join();
*/
    
    std::cout << "Dividing... " << std::flush;
    cv::Mat out(m[0].rows, m[0].cols, CV_64FC3);
    out.setTo(cv::Scalar(0, 0, 0, 0));

    // ISSUE std::ref not behaving as expected when calling [accumulate].
    // Workaround of m[0].convertTo(...) instead of out.convertTo(...) is
    // inefficient (discards threading advantages).
    for (auto e : m) out += e;                        // Accumulate the arrays from the threads into a single array
    m[0].convertTo(out, CV_8U, 1. / images.size()); // Convert back to CV_8U type, applying the division to get the actual mean
    std::cout << bright+green+"done"+res+"." << std::endl;
    
    return out;
}
cv::Mat median_coadd(const std::vector<cv::Mat> &images) {
// Takes the median at each pixel for a stack of images. Assumes the images
// are aligned and that they are the same size.
    const int nthreads = max_threads > images[0].rows * images[0].cols ? 1 : max_threads;
    std::vector<cv::Mat> _result(3, cv::Mat::zeros(images[0].rows, images[0].cols, CV_8UC1));

    // Calculate the size of the section of the image to be processed by each thead
    size_t block = ceil(images[0].rows * images[0].cols / (double)nthreads);
    size_t offset = 0;
    size_t tidx = 1;

    // Initialize the threads and pass appropriate arguments to each one
    std::cout << "Computing medians..." << std::endl;
    std::vector<std::thread> threads(nthreads);
    for (long ii = 0; ii < nthreads - 1; ii ++, offset += block) {
        threads[ii] = std::thread(_median_coadd, tidx++, images, std::ref(_result), offset, (offset + block) * 3);
    }
    threads.back() = std::thread(_median_coadd, tidx++, images, std::ref(_result), offset, images[0].rows * images[0].cols * 3);

    for (auto &t : threads) t.join();

    cv::Mat result;
    cv::merge(_result, result);
    
    return result;
}
void _median_coadd(const int tidx, const std::vector<cv::Mat> &images, std::vector<cv::Mat> &output, size_t offset, const size_t end) {
// Abstraction for parallel execution of [median()]. Hard coded for use with 3-channel input 
// only (pointer stuff would get too complicated otherwise).
//    std::cout << "Thread "+std::to_string(tidx)+" started. Region: "+std::to_string(offset)+" through "+std::to_string(end)+"\n" << std::flush;
    const size_t n = images.size();
    std::vector<uchar> pixels(n);   // Initialize vector to store pixel values
    const bool even = (n % 2);

    // Array of pointers to the start of this thread's block of the output
    std::array<uchar*, 3> start;
    for (auto ii = 0; ii < 3; ii ++) start[ii] = output[ii].ptr(0, 0) + offset;

    // Pointers to the starts of the input images
    std::vector<uchar*> img_ptrs(n);
    for (size_t ii = 0; ii < n; ii ++) img_ptrs[ii] = (uchar*)images[ii].ptr(0, 0);

    size_t im;
    size_t total = images[0].rows * images[0].cols;
    for (offset; offset < end; offset += 3) {
        print_percent(progress++, total);
        for (size_t c = 0; c < 3; c ++, im = 0) {
            // Extract relevant pixel info for this location and channel
            for (auto &p : pixels) p = *(img_ptrs[im++] + offset + c);

            // Sort pixel values for this channel and location accross all images
            std::sort(std::begin(pixels), std::end(pixels), [](const uchar &l, const uchar &r) { return l < r; }); 
            if (even) *(start[c]) = pixels[n / 2 - 1] / 2 + pixels[n / 2] / 2;      // Unusual order of operations is to prevent uchar overflow
            else *(start[c]) = pixels[n / 2];
            
            start[c] ++;        // Can't have this in the loop syntax because it will produce UB
        }
    }
    std::cout << std::endl;
}

std::vector<cv::Mat> scrub_hot_pixels(const std::vector<cv::Mat> images) {
    cv::Mat m = images[0];
    const cv::Vec3b zero(0, 0, 0);
    int idx = 0;
    long hot = 0;
    std::cout << "Scrubbing hot pixels..." << std::endl;
    for (auto image : std::vector<cv::Mat>(images.begin() + 1, images.end())) {
        if (image.rows != m.rows || image.cols != m.cols) {
            std::cout << "Error, shape mismatch on image with shape " << image.rows << "x" << image.cols;
            std::cout << " expected " << m.rows << "x" << m.cols << std::endl;
            return images;
        }
#pragma omp parallel for schedule(dynamic)
        for (int r = 0; r < m.rows; r ++) {
            for (int c = 0; c < m.cols; c ++) {
                if (m.at<cv::Vec3b>(r, c) != zero && (m.at<cv::Vec3b>(r, c) - image.at<cv::Vec3b>(r, c)) < 5) {
                    m.at<cv::Vec3b>(r, c) = zero;
                }
            }
        }
        print_percent(idx++, images.size() - 1);
    }
#pragma omp parallel for schedule(dynamic)
    for (int r = 0; r < m.rows; r ++) {
        for (int c = 0; c < m.cols; c++) {
            if (m.at<cv::Vec3b>(r, c) != zero) {
                #pragma omp critical
                    hot++;
            }
        }
    }
    std::cout << "Found " << hot << " hot pixels." << std::endl;
    cv::imwrite("./hot_pixels.tif", m);
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < images.size(); i ++) {
        images[i] -= m;
    }
    return images;
}

cv::Mat4b advanced_coadd(const std::vector<cv::Mat> images, double threshold) {
// BROKEN
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
    progress = 0;
    for (auto image : images) {
        long transparent = 0;
        print_percent(progress++, images.size());
        cv::Mat current(image.rows, image.cols, CV_32FC4);
        cv::cvtColor(image, current, cv::COLOR_BGR2BGRA);
        std::cout << type2str(current.type()) << std::endl;
#pragma omp parallel for schedule(dynamic)
        for (int r = 0; r < current.rows; r ++) {
            for (int c = 0; c < current.cols; c ++) {
                cv::Vec4b& pixel = current.at<cv::Vec4b>(r, c);
                if (!brighter_than(pixel, max*threshold)) {
                    pixel[3] = 0.0;
                    transparent ++;
                }
            }
        }
        current.convertTo(current, CV_32FC4);
        modified.push_back(current);
        if (transparent) {
//            std::cout << transparent << " pixels below threshold." << std::endl;
        }
        else {
//            std::cout << "No pixels detected below threshold" << std::endl;
        }
    }
    std::cout << "Accumulating..." << std::endl; 
    for (int ii = 0; ii < modified.size(); ii ++) {
        modified[ii].convertTo(modified[ii], CV_32FC4);
        std::cout << type2str(modified[ii].type()) << std::endl;
        for (int r = 0; r < m.rows; r ++) {
            for (int c = 0; c < m.cols; c ++) {
                cv::Vec4b pixel = modified[ii].at<cv::Vec4b>(r, c);
                cv::Vec4b& mpixel = m.at<cv::Vec4b>(r, c);
                if (pixel[3] != 0.0) {
                    mpixel += pixel;
                    mask[r][c] += 1;
                }                
            }
        }
    }
    cv::imwrite("temp.png", m);
    std::cout << "Dividing... " << std::endl;
    for (int ii = 0; ii < m.rows; ii ++) {
        print_percent(ii, m.rows);
        for (int jj = 0; jj < m.cols; jj ++) {
            m.at<cv::Vec4b>(ii, jj) = m.at<cv::Vec4b>(ii, jj) / (double)mask[ii][jj];
        }
    }
    
    std::cout << "Converting... " << std::flush;
    cv::Mat4b output(m.rows, m.cols, CV_32FC4);
//    cv::cvtColor(m, output, cv::COLOR_BGRA2BGR);
    m.convertTo(output, CV_8U, 1. / modified.size());
    std::cout << bright+green+"done."+res << std::endl;
    return output;
}

void align_stars(cv::Mat &anc, cv::Mat &com, cv::Mat &result) {
    // Do a downhill minimization by repeatedly sampling the translation-rotation-pespective
    // parameter space and iteratively refining. This method works best for images with dim or 
    // no foreground and little star trailing. For images that do not satisfy this, [align_images]
    // should be used instead.
    // {anc} - anchor image
    // {com} - comparison image
    // {result} - result after alignment

    // NOTE Should be arguments
    const size_t n = 25; 
    const size_t iters = 4;

    // Extract a mask of only the brightest stars (within 0.5% of max brightness)
    const cv::Mat anc_stars = brightness_find_legacy(anc, find_max({anc}), 2);
    const cv::Mat com_stars = brightness_find_legacy(com, find_max({com}), 2);

    // Convert star masks into vector of positions
    const std::vector<std::pair<double, double>> anc_pos = star_positions(anc_stars, 100);
    const std::vector<std::pair<double, double>> com_pos = star_positions(com_stars, 100);

    // Track the location and value of the minimum difference
    double min = diff(anc_pos, com_pos);
    std::tuple<double, double, double> where = std::make_tuple(0, 0, 0);

    // Keep track of the size of the current parameter subspace.
    double rot_it = Pi*2;
    double x_it = (anc.cols + com.cols);
    double y_it = (anc.rows + com.rows);

    std::cout << "Aligning by star field..." << std::endl;
    // Do the iteratively refined minimization
    for (size_t ii = 0; ii < iters; ii ++, rot_it /= n, x_it /= n, y_it /=n) {

        // Reduce the search space according to the location of the minimum in the previous iteration
        std::pair<double, double> rot_lim = std::make_pair(-rot_it / 2.0, rot_it / 2.0) + std::get<0>(where);
        std::pair<double, double> x_lim = std::make_pair(-x_it / 2.0, x_it / 2.0) + std::get<1>(where);
        std::pair<double, double> y_lim = std::make_pair(-y_it / 2.0, y_it / 2.0) + std::get<2>(where);

        // Would prefer the loop iterators be floating point instead of storing the whole vector, but OMP doesn't work like that :(
        std::vector<double> _rot = linspace(rot_lim, n);
        std::vector<double> _x = linspace(x_lim, n);
        std::vector<double> _y = linspace(y_lim, n);

        for (const auto &rot : _rot) {
            for (const auto &x : _x) {
                for (const auto &y : _y) {
                    double _diff = diff(anc_pos, rotate(com_pos, rot) - std::make_pair(x, y));
                    if (_diff < min) {
                        min = _diff;
                        where = std::make_tuple(rot, x, y);
                    }
                }
            }
        }
        print_percent(ii, iters);
    }
    std::cout << std::get<0>(where) << " " << std::get<1>(where) << " " << std::get<2>(where) << std::endl;
    std::cout << min << std::endl;
}

void align_images(cv::Mat &im1, cv::Mat &im2, cv::Mat &im1Reg) {
    // Performs feature based alignment of {im1} to {im2}.
    cv::Mat im1Gray, im2Gray, h;

    // Convert images to grayscale
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
    
    std::vector<double> separations = find_separations(matches, keypoints1, keypoints2);
    
    // Sort matches by score
    std::vector<std::pair<double, cv::DMatch> > paired;
    for (int ii = 0; ii < matches.size(); ii ++) {
        std::pair<double, cv::DMatch> p(separations[ii], matches[ii]);
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
double find_separation(cv::DMatch m, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2) {
    return sqrt(pow(kp1[m.queryIdx].pt.x - kp2[m.trainIdx].pt.x, 2) + pow(kp1[m.queryIdx].pt.y - kp2[m.trainIdx].pt.y, 2));
}

std::vector<double> find_separations(std::vector<cv::DMatch> matches, std::vector<cv::KeyPoint> kp1, std::vector<cv::KeyPoint> kp2) {
    std::vector<double> separations;
    for (const auto &match : matches) {
        separations.push_back(find_separation(match, kp1, kp2));
    }
    return separations;
}

void interpolate_simple(cv::Mat &im, const Blob &blob) {
    
}

// Extract blobs from a binary image mask in CV_8U1C colorspace
std::vector<Blob> blob_extract(const cv::Mat &mask) {
    std::vector<Blob> blobs;
    uchar* pixel;
    uchar* start = const_cast<uchar*>(mask.ptr(0, 0));

    // Iterate using pointer arithmetic for efficiency (assumes contiguous storage)
    for (pixel = start; pixel < mask.ptr(mask.rows - 1, mask.cols - 1); pixel ++) {
        if (*pixel) {  // If the current pixel value is non-zero
            Blob blob;
            _blob_extract(mask, blob, pixel, start);
            blobs.push_back(blob);
        }
    }
    
    return blobs;
}

// Recursive function to extract a single blob given an image mask and a starting pixel 
void _blob_extract(const cv::Mat &mask, Blob &blob, uchar* pixel, uchar* start) {

    Pos p { (pixel - start) / mask.cols, (pixel - start) % mask.cols };
    if (*pixel) {
        blob.to_blob(p);
        _blob_extract(mask, blob, pixel + 1, start);
        _blob_extract(mask, blob, pixel + mask.rows, start);
    }
    else blob.to_perim(p);
    
}

cv::Mat median_filter(const cv::Mat &image, const FilterMode mode, const bool norm, const bool stretch, 
                      const size_t _kernel, const long smoothing, const long jitter, const double filter_strength) {
    cv::Mat out(image.rows, image.cols, image.type());
    const size_t nb = image.channels();

    // TODO Implement rolling-window filtering for row, column, and local filtering.
    //      Calculate the median in each kernel, and add 1 / kernel_size * median to 
    //      a running total. Subtract the running total at the end for smooth median
    //      subtraction. 

    std::mt19937 gen;
    std::uniform_int_distribution<> jitterer = prng((_kernel - 1) < jitter ? -jitter : -(_kernel - 1), jitter, gen);
    size_t kernel = _kernel + jitterer(gen);

    if (mode == FilterMode::global) {
        if (norm) {
            for (int b = 0; b < nb; b ++) {
                cv::Mat channel(image.rows, image.cols, CV_8UC1);
                cv::extractChannel(image, channel, b);
                Chunk chunk = gaussian_estimate(channel.ptr(0, 0), channel.cols, Extent { 0, channel.cols, 0, channel.rows });

                uchar* pixel = out.ptr(0, 0) + b;
                for (size_t rc = 0; rc < channel.total(); rc ++, pixel += nb) {
                    double filter = chunk.median * filter_strength;
                    *pixel = channel.data[rc] > filter ? channel.data[rc] - filter : 0;
                }
            }
        }
        else {
            Chunk chunk = gaussian_estimate(image.ptr(0, 0), image.cols, Extent { 0, image.cols, 0, image.rows });
            uchar* pixel = out.ptr(0, 0);
            for (size_t rc = 0; rc < image.rows * image.cols * nb; rc ++, pixel ++) {
                *pixel = image.data[rc] > chunk.median ? image.data[rc] - chunk.median : 0;
            }
        }
    }
    else if (mode == FilterMode::row) {
        uchar* pixel;
        cv::Mat channel(image.rows, image.cols, CV_8UC1);
        for (int b = 0; b < nb; b ++) {
            cv::extractChannel(image, channel, b);
            std::vector<Chunk> chunks;
            std::vector<size_t> kernels;

            for (long r = 0; r < channel.rows; r += kernel) {
                chunks.push_back(gaussian_estimate(channel.ptr(r, 0), channel.cols, Extent { 0, channel.cols, 0, (kernel + r < channel.rows ? (long)kernel : channel.rows - r) }));
                kernel = _kernel + jitterer(gen);
                kernels.push_back(kernel);
            }
            smooth_median(chunks, smoothing);

            for (long r = 0, ii = 0; r < channel.rows; r += kernels[ii], ii ++) {
                pixel = out.ptr(r, 0) + b;
                for (size_t rc = r * channel.cols; rc < channel.cols * (kernels[ii] + r) && rc < channel.total(); rc ++, pixel += nb) {
                    *pixel = channel.data[rc] > chunks[ii].median ? channel.data[rc] - chunks[ii].median : 0;
                }
            }
        }
    }
    else if (mode == FilterMode::col) {
        Chunk chunk;
        cv::Mat channel(image.rows, image.cols, CV_8UC1);
        for (int b = 0; b < nb; b ++) {
            cv::extractChannel(image, channel, b);
            std::vector<Chunk> chunks;
            std::vector<size_t> kernels;
            
            for (long c = 0; c < channel.cols; c += kernel) {
                chunks.push_back(gaussian_estimate(channel.ptr(0, c), channel.cols, Extent { 0, (kernel + c < channel.cols ? (long)kernel : channel.cols - c), 0, channel.rows }));
                kernel = _kernel + jitterer(gen);
                kernels.push_back(kernel);
            }    
            smooth_median(chunks, smoothing);

            for (long c = 0, ii = 0; c < channel.cols; c += kernels[ii], ii ++) {
                for (size_t _r = 0; _r < channel.rows; _r ++) {
                    for (size_t _c = c; _c < c + kernels[ii]; _c ++) { 
                        *(out.ptr(_r, _c) + b) = channel.data[_r * channel.cols + _c] > chunks[ii].median ? channel.data[_r * channel.cols + _c] - chunks[ii].median : 0;
                    }
                }
            }
        }
    }
    else if (mode == FilterMode::rowcol) {
        out = median_filter(image, FilterMode::row, kernel);
        normalize(out, 0.75);
        out = median_filter(out, FilterMode::col, kernel);
    }
    else if (mode == FilterMode::colrow) {
        out = median_filter(image, FilterMode::col, kernel);
        normalize(out, 0.75);
        out = median_filter(out, FilterMode::row, kernel);
    }

    if (stretch) {
        // Calculate stretch factor
        uchar max = find_max({out});
        double factor = 255.0 / double(max);
        
        uchar* pixel = out.ptr(0, 0);
        for (size_t rc = 0; rc < out.rows * out.cols * nb; rc ++, pixel ++) {
            *pixel *= factor;
        }
    }
    
    return out;
}

void normalize(cv::Mat &image, const double &c) {
    double min, max;
    double tot;
    for (int b = 0; b < image.channels(); b ++) {
        cv::Mat channel(image.rows, image.cols, CV_8UC1);
        cv::extractChannel(image, channel, b);
        cv::minMaxLoc(channel, &min, &max);
        tot += max;
    }
    tot /= 3.0;
    double f = (c * 255) / tot;

    uchar* pixel = image.ptr(0, 0);
    for (size_t rcb = 0; rcb < image.total() * image.channels(); rcb ++, pixel ++) {
        *pixel *= f;
    }
}
void smooth_median(std::vector<Chunk> &chunks, const long smoothing) {
    if (chunks.empty() || smoothing == 0) return; 
    std::vector<Chunk> out(chunks.size());

    // Smooth medians of chunk vector based on rolling average of 2 * {smoothing} + 1    
    size_t n = 0;
    long sum = 0;
    for (long c = 0; c < chunks.size(); c ++) {
        sum = 0;
        n = 0;
        for (long s = c - smoothing; s < c + smoothing; s ++) {
            if (s > 0 && s < chunks.size()) {
                sum += chunks[s].median;
                n ++;
            }
        }
        out[c].median = sum / n;
    }
    chunks = out;
}
