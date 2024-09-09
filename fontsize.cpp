#include <opencv2/opencv.hpp>
#include <iostream>

void saveAsWebP(const cv::Mat& image, const std::string& output_file, int quality) {
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
    compression_params.push_back(quality);

    if (!cv::imwrite(output_file, image, compression_params)) {
        std::cerr << "Error saving image as WebP.\n";
    }
}

int main() {
    cv::Mat img = cv::imread("example_image.png");
    if (img.empty()) {
        std::cerr << "Error loading image\n";
        return 1;
    }

    saveAsWebP(img, "compressed_image.webp", 80); // Save with 80% quality

    return 0;
}
