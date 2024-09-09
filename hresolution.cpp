#include <opencv2/opencv.hpp>
#include <iostream>

void saveHighResImage(const cv::Mat& image, const std::string& output_file) {
    // Example with double the resolution
    cv::Mat high_res_img;
    cv::resize(image, high_res_img, cv::Size(image.cols * 2, image.rows * 2));

    if (!cv::imwrite(output_file, high_res_img)) {
        std::cerr << "Error saving high resolution image.\n";
    }
}

int main() {
    cv::Mat img = cv::imread("example_image.png");
    if (img.empty()) {
        std::cerr << "Error loading image\n";
        return 1;
    }

    saveHighResImage(img, "high_res_image.png");

    return 0;
}
