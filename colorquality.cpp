#include <opencv2/opencv.hpp>
#include <iostream>

void saveWithColorProfile(const cv::Mat& image, const std::string& output_file) {
    cv::imwrite(output_file, image); // Ensure the image format supports color profiles
}

int main() {
    cv::Mat img = cv::imread("example_image.png", cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        std::cerr << "Error loading image\n";
        return 1;
    }

    saveWithColorProfile(img, "image_with_profile.tiff");

    return 0;
}
