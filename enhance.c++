#include "enhance.h"

cv::Mat read_image(std::string file) {              // Read the image file specified by {file}
    if (!fs::exists(fs::path(file))) {                // If the file does not exist simply exit with uninitialized return
        std::cout << "Error - file "+file+" does not exist or other file error." << std::endl;
        return cv::Mat();    
    }

    cv::Mat image = cv::imread(file, cv::IMREAD_COLOR);
    return image;
}

std::vector<cv::Mat> read_images(std::vector<std::string> files) { // Read the image files in the string vector {files}
    std::mutex mtx;                                                  // Create a mutex for pushing images onto {images}
    if (files.empty()) return std::vector<cv::Mat>();              // To avoid segmentation fault in case of empty filelist, 
                                                                     // return default-constructed vector of [cv::Mat] objects
    std::vector<cv::Mat> images;                                   // Initialize new vector of [cv::Mat] objects
    std::cout << "Reading files..." << std::endl;
    int count = 0;
#pragma omp parallel for schedule(dynamic)
    for (int ii = 0; ii < files.size(); ++ii) {                      // Then for every file in the list
        cv::Mat image;                                               // create a new temporary [cv::Mat] object,
        image = cv::imread(files[ii], cv::IMREAD_COLOR);             // read the {ii}th file from {files} into the temp 
        mtx.lock();
        if (!image.empty()) {                                        // object. If it is not empty
            images.push_back(image);                                 // Push it onto the images vector
        }
        else {                                                       // Or if file does not open, print a message and skip 
            std::cout << "Could not open " << yellow << files[ii] << res << " - file may not exist." << std::endl;
        }
        print_percent(count++, files.size());
        mtx.unlock();
    }
    return images;                                                   // Then return the [cv::Mat] vector of images
}

cv::Mat star_trail(const std::vector<cv::Mat> images, const uint threshold) {
    if (images.empty()) return cv::Mat();                            // If the images list is empty, return an empty cv::Mat object
    int rows = images[0].rows;
    int cols = images[0].cols;

    cv::Mat image = coadd(images);
    cv::Mat layer;

    std::cout << "Accumulating star trails..." << std::endl;
    for (size_t img = 0; img < images.size(); ++img) {                 // Loop through every image in the vector {images}
        images[img].convertTo(layer, CV_8UC3);

        // Apply the 'lighten-only' filter
        auto* layer_pixel = layer.ptr<cv::Vec3b>(0, 0);
        auto* image_pixel = image.ptr<cv::Vec3b>(0, 0);
        for (size_t rc = 0; rc < image.rows * image.cols; rc++, layer_pixel++, image_pixel++) {
            if (brightness(*layer_pixel) > threshold && brightness(*layer_pixel) > brightness(*image_pixel)) {
                *image_pixel = *layer_pixel;
            }
        }
        print_percent(img, images.size());
    }
    return image;
}

cv::Mat brightness_find(const cv::Mat &_image, const size_t z) {
    // First convert image to grayscale and set up binary output
    cv::Mat image(_image.rows, _image.cols, CV_8UC1);
    cvtColor(_image, image, cv::COLOR_BGR2GRAY);
    cv::Mat starmask = cv::Mat::zeros(_image.rows, _image.cols, CV_8UC1);
    
    // Retrieve statistical information from entire image
    std::cout << "Retrieving image-wide statistics... " << std::flush;
    Chunk chunk = gaussian_estimate(image.ptr(0, 0), image.cols, Extent { 0, image.cols, 0, image.rows });
    std::cout << bright+green+"done"+res+"." << std::endl;

    // Iterate over the image using pointer arithmetic
    uchar* ipixel = image.ptr(0, 0);
    uchar* opixel = starmask.ptr(0, 0);
    std::cout << "Extracting stars based on mean of " << std::fixed << std::setprecision(2) << chunk.mean << ", standard deviation of " << std::setprecision(2) << chunk.std;
    std::cout << ", and z-score threshold of " << z << "... " << std::flush;
    for (size_t rc = 0; rc < image.cols * image.rows; rc ++, ipixel ++, opixel ++) {
        if (*ipixel - chunk.mean > z * chunk.std) *opixel = 255;
    }
    std::cout << bright+green+"done"+res+"." << std::endl;

    return starmask;
}

cv::Mat brightness_find_legacy(const cv::Mat &image, uchar max_intensity, 
                            int nn, double star_threshold) {                     // Find stars in the given image and return a mask of stars
    cv::Mat out = cv::Mat::zeros(image.rows, image.cols, CV_64FC3);            // Initialize output object of type CV_64FC3

    cv::Vec3b* pixel = const_cast<cv::Vec3b*>(image.ptr<cv::Vec3b>(0, 0));
    for (size_t ii = 0; ii < out.rows; ii++) {                                   // Then loop through every pixel
        for (size_t jj = 0; jj < out.cols; jj++, pixel++) {                      // Iterate the data pointer only with the fastest loop
            if (brightness(*pixel) > max_intensity*star_threshold &&             // If the brightness is greater than the star threshold
                brightness(*pixel) > brightness(out.at<cv::Vec3b>(ii, jj))) {    // and greater than the current output brightness at that location
                out.at<cv::Vec3b>(ii, jj) = *pixel;                              // then apply the new pixel values to the output image.
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
    return out;                                                                  // Return a mask with stars isolated
}

cv::Mat gaussian_find(const cv::Mat &_image, long w, size_t z) {
/* 
   Extract a binary mask of stars from an image by using localized analysis of 
   mean and standard deviation to decide if a pixel is likely part of a star or not

     {_image} - The input image
     {w}      - The bandwidth so to speak, equal to the maximum possible 
                value for half the sidelength of the ROI. NOTE - {w} is `long`
                instead of `size_t` because it eliminates the need for signed-cast
                when comparing r - w > 0 and c - w > 0
     {z}      - The z-score threshold to use for filtering. Defaults to 12
*/
    // First convert image to grayscale and set up binary output
    cv::Mat image(_image.rows, _image.cols, CV_8UC1);
    cvtColor(_image, image, cv::COLOR_BGR2GRAY);
    cv::Mat out = cv::Mat::zeros(_image.rows, _image.cols, CV_16FC1);
    
    // Iterating pointers is much faster than using [.at<>()]
    uchar* pixel = image.ptr(0, 0);
    uchar* _out = out.ptr(0, 0);

    // Set up 2D Matrix to store mean and standard deviation from each 'chunk'. We will proceed 
    // by precalculating these for each chunk and then aggregating the values from the portions 
    // of the chunks overlapped by the ROI to decide whether or not the current pixel is part of 
    // a star. This changes the operation from O(2 * w * _image.rows * _image.cols) to O(6 * 
    // _image.rows * _image.cols / w + _image.rows * _image.cols) - a dramatic reduction for 
    // {w} > 2.
    long dw = w * 2;
    size_t _rows = std::ceil(_image.rows / (double)dw);        // Explicit `double` cast is required for ceil to work how we want
    size_t _cols = std::ceil(_image.cols / (double)dw);        // without it the integer division results in premature truncation
    Matrix<Chunk> chunks(_rows, _cols);
    Chunk chunk;

    std::cout << "Precalculating chunk means... " << std::endl;
    for (long r = 0; r < _rows; r ++, pixel += dw * (image.cols -  _cols)) {    // Have to iterate {pixel} on both loops to account for the y-dimension of the chunks
        print_percent(r, _rows);
        for (long c = 0; c < _cols; c ++, pixel += dw) {
            chunks(r, c) = gaussian_estimate(pixel, image.cols, Extent {(c * dw - w > 0 ? w : c * dw), (c * dw + w < image.cols ? w : image.cols - c * dw - 1),
                                                                        (r * dw - w > 0 ? w : r * dw), (r * dw + w < image.rows ? w : image.rows - r * dw - 1)});
            chunks(r, c).pos = Pos {r, c};
        }
    }

    std::cout << "Extracting stars based on chunksize " << w << "x" << w << " and z-score threshold of " << z << "... " << std::endl;
    long r, c;
    double _r = 0, _c = 0, _it = 1.0 / dw;        // Set up slower iterators to convert between {image} iteration and {chunk} iteration

    for (r = 0, pixel = image.ptr(0, 0), _out = out.ptr(0, 0); r < image.rows; r ++, _r += _it) {
        print_percent(r, image.rows);
        for (c = 0, _c = 0; c < image.cols; c ++, pixel ++, _out ++, _c += _it) {
        
            // Get the chunk the current pixel is in 
            chunk = chunks((size_t)_r, (size_t)_c);

            // And then combine with the nearest neighbor chunks (no diagnoals)
            if (_c > 0) chunk += chunks((size_t)_r, (size_t)(_c - 1));
            if (_r > 0) chunk += chunks((size_t)(_r - 1), (size_t)_c);
            if (_c < _cols - 1) chunk += chunks((size_t)_r, (size_t)(_c + 1));
            if (_r < _rows - 1) chunk += chunks((size_t)(_r + 1), (size_t)_c);

            // Not taking the absolute value of the difference here because we only want 
            // pixels that are {z} standard deviations brighter than their surroundings
            if (*pixel - chunk.mean > z * chunk.std) *_out = 100;
            // NOTE Could filter out blobs below a certain extent here, don't necessarily need another loop
        }
    }
    
    return out;
}
Chunk gaussian_estimate(const uchar* pixel, const size_t &cols, const Extent &e) {
// Get the mean and variance of a region of interest (ROI) determined by the bandwidth in [gaussian_find]
    Chunk chunk(e);

    // Find the mean and median brightness of the region of interest (ROI)
    std::vector<double> pixels;
    for (long _r = -e.b; _r < e.t; _r++) {
        for (long _c = -e.l; _c < e.r; _c++) { 
            chunk.mean += *(pixel + _c + _r * cols);
            chunk.n ++; // Accumulate instead of calculating because center-defined Extents would have off-by-one otherwise
            
            pixels.push_back((double)*(pixel + _c + _r * cols));
        }
    }
    chunk.mean /= chunk.n;
    chunk.median = median(pixels);

    // Find the standard deviation of the region of interest (ROI)
    for (long _r = -e.b; _r < e.t; _r++) {
        for (long _c = -e.l; _c < e.r; _c++) chunk.std += abs(*(pixel + _c + _r * cols) - chunk.mean);
    }
    chunk.std /= chunk.n;
    chunk.var = std::pow(chunk.std, 2);
    chunk.n;

    return chunk;
}

cv::Mat depollute(cv::Mat &image, const size_t size, const size_t z, const FindBy find) {

    cv::Mat stars;
    if (find == FindBy::gaussian) stars = gaussian_find(image, size, z);
    else if (find == FindBy::brightness) stars = brightness_find(image, z);
    cv::imwrite("starmask.tif", stars);

    std::cout << "Modeling and removing light pollution and sky glow... " << std::endl;
    const int nbands = image.channels();
    std::valarray<Eigen::MatrixXd> _model(Eigen::MatrixXd::Zero(image.rows, image.cols), 3);
    for (long r = 0; r < image.rows; r += size) {
        print_percent(r, image.rows);
        for (long c = 0; c < image.cols; c += size) {
            for (int b = 0; b < nbands; b ++) {
                _model[b].block(r, c, size, size) = depollute_region(image, stars, r, c, b, size);
            }
        }
    }
    if (image.rows % size != 1) print_percent(image.rows - 1, image.rows);

    // Return the model as an image
    std::cout << "Creating reference image of modeled values... " << std::endl;
    cv::Mat model(image.rows, image.cols, image.type());
    for (long r = 0; r < image.rows; r ++) {
        print_percent(r, image.rows);
        for (long c = 0; c < image.cols; c ++) {
            for (int b = 0; b < image.channels(); b ++) {
                *(model.ptr(r, c) + b) = _model[b](r, c);
            }
        }
    }
    
    return model;
}
Eigen::MatrixXd depollute_region(cv::Mat &image, const cv::Mat &stars, const long &r, const long &c, const int &b, const size_t &size) {

    // Avoid overflowing the image bounds by downscaling row and column sizes as needed 
    const size_t c_size = c + size < image.cols ? size : image.cols - c;
    const size_t r_size = r + size < image.rows ? size : image.rows - r;

    Eigen::MatrixXd W = Eigen::MatrixXd::Zero(c_size*r_size, c_size*r_size);
    
    Eigen::MatrixXd X = Eigen::MatrixXd::Constant(c_size*r_size, 3, 1);
    Eigen::MatrixXd Y(c_size*r_size, 1);

    // Populate the weighted least squared components
    size_t idx = 0;
    for (long ii = 0; ii < r_size; ii ++) {
        for (long jj = 0; jj < c_size; jj ++, idx ++) {
            if ((int)*(stars.ptr(ii + r, jj + c) + b)) continue;
            W(idx, idx) = 1.0 / (c_size * r_size);
            X(idx, 0) = jj;
            X(idx, 1) = ii;
            Y(idx) = (int)*(image.ptr(ii + r, jj + c) + b);
        }
    }

    // Calculate and apply the coefficient matrix to retrieve the predicted pixel values
    Eigen::MatrixXd _XB = X * ((X.transpose() * W * X).inverse() * (X.transpose() * W * Y));
    Eigen::MatrixXd XB(r_size * c_size, 1);

    // Subtract off the modeled light pollution/sky glow
    idx = 0;
    uchar* pixel = image.ptr(0, 0);
    for (long ii = 0; ii < r_size; ii ++) {
        for (long jj = 0; jj < c_size; jj ++, idx ++) {
            pixel = image.ptr(ii + r, jj + c) + b;
            
            if ((int)*(stars.ptr(ii + r, jj + c) + b)) {
                XB(idx) = 0;
            }
            else {
                XB(idx) = _XB(idx);
                
                if (XB(idx) < *pixel) *pixel -= XB(idx);
                else *pixel = 0;
            }
        }
    }

    return Eigen::Map<Eigen::MatrixXd>(XB.data(), r_size, c_size).transpose();
}

std::vector<std::pair<double, double>> star_positions(const cv::Mat &starmask, const size_t &n) {
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

int brightness(const cv::Vec3b& input) {                     // Find and return the brightness [0-255] of the three 
    return (input.val[0] + input.val[1] + input.val[2])/3;   // channel pixel {input}
}

bool brighter_than(cv::Vec4b pixel, double threshold) { return ((pixel[0] + pixel[1] + pixel[2]) / 3 > threshold); }

uchar find_max(std::vector<cv::Mat> images) {
    if (images.empty()) return 0;

    if (images.size() > 1) std::cout << "Finding maximum intensity..." << std::endl;
    uchar max_intensity = 0;
    for (int img = 0; img < images.size(); ++img) {           // Loop through every image in the vector {images}
        print_percent(img, images.size());                    // print the percent complete
        for (int ii = 0; ii < images[img].rows; ii ++) {      // Then loop through every pixel in each image and look for stars
            for (int jj = 0; jj < images[img].cols; jj ++) {
                int value = brightness(images[img].at<cv::Vec3b>(ii, jj));
                                                              // Pull the brightness value for the current pixel
                if (value > max_intensity) {                  // And if it is brighter than the previous max
                    max_intensity = value;                    // Replace the previous max with the new brightness value
                }
            }
        }
    }
    return max_intensity;                                     // Then return the maximum brightness value
}
    
std::vector<cv::Mat> extract_frames(std::vector<std::string> files) {  // Extract all frames of given video files to a single
                                                                       // vector of [cv::Mat] objects
    if (files.empty()) return std::vector<cv::Mat>();                  // Guard against seg fault on empty list with return catch
    
    std::vector<cv::Mat> frames;                                       // Initialize a new vector of [cv::Mat] objects
    std::cout << "Extracting frames..." << std::endl;
    frames = read_video(files);
    return frames;                                                     // Return the [cv::Mat] vector {frames} when all files have been
}                                                                      // read in.

std::vector<cv::Mat> read_video(std::vector<std::string> videos) {
    /* -    -    -    -    -    -    -    -    - 
       This works *most* of the time....
       -    -    -    -    -    -    -    -    - */
    std::vector<cv::Mat> frames;

    AVFormatContext* informat_ctx = NULL;
    AVFrame* frame = NULL;
    AVFrame* decframe = NULL;
    AVCodecContext* inav_ctx = NULL;
    AVCodec* in_codec = NULL;
    int valid_frame = 0;
    int video_stream = -1;
    frame = av_frame_alloc();
    decframe = av_frame_alloc();

    av_register_all();
    avcodec_register_all();
    
    int ret;
    int nframe = 0;

    for (int ii = 0; ii < videos.size(); ii++) {                        // For every file in the filelist {videos}, attempt to open
        if ((ret = avformat_open_input(&informat_ctx, videos[ii].c_str(), NULL, NULL)) < 0) {
        	// If opening fails print error message and return default constructed `vector` of `cv::Mat`
        	char errbuf[1024];
        	av_strerror(ret, errbuf, 1024);
            std::cout << "Could not open file "+yellow << videos[ii] << res+": " << errbuf << std::endl;
            exit(-1);
        }

        // Read the meta data 
        ret = avformat_find_stream_info(informat_ctx, 0);
        if (ret < 0) {
            std::cout << red << "Failed to read input file information. " << res << std::endl;
            exit(-1);
        }

        for(ii = 0; ii < informat_ctx->nb_streams; ii ++) {
            if (informat_ctx->streams[ii]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video_stream < 0) {
                video_stream = ii;
            }
        }
        if (video_stream == -1) {
            std::cout << "Could not find stream index." << std::endl;
            exit(-1);
        }

        in_codec = avcodec_find_decoder(informat_ctx->streams[video_stream]->codec->codec_id);
        if (in_codec == NULL) {
            std::cout << "Could not find codec: " << avcodec_get_name(informat_ctx->streams[video_stream]->codec->codec_id) << std::endl;
            exit(1);
        }
        inav_ctx = informat_ctx->streams[video_stream]->codec;

        avcodec_open2(inav_ctx, in_codec, NULL);                        // Open the input codec

        std::vector<uint8_t> framebuf(avpicture_get_size(inav_ctx->pix_fmt, inav_ctx->width, inav_ctx->height));
        avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), AV_PIX_FMT_BGR24, inav_ctx->width, 
                       inav_ctx->height);
                       
        struct SwsContext *img_convert_ctx;
        while (true) {
            AVPacket pkt;
            av_init_packet(&pkt);
            std::cout << '\r' << cyan << ii << res << " | Frame: " << magenta;
            std::cout << std::setw(4) << std::setfill('0') << nframe++ << res;
            ret = av_read_frame(informat_ctx, &pkt);                     // Read the next frame in
            avcodec_decode_video2(inav_ctx, frame, &valid_frame, &pkt);  // Decode the next frame
            if (!valid_frame) continue;                                  // Ignore invalid frames   
            if (ret == 0) {                                              
                img_convert_ctx = sws_getContext(
                    inav_ctx->width,
                    inav_ctx->height,
                    AV_PIX_FMT_BGR24,
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
                frames.push_back(_frame);                                // And add it to the output vector
            }
            else {
                break;                                                   // Once an invalid frame is read, break out of loop
            }
        }
        std::cout << std::endl;
    }
    return frames;                                                       // And return 
}

