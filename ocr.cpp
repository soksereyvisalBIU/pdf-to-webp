#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <iostream>

void performOCR(const cv::Mat& image, const std::string& output_text_file) {
    tesseract::TessBaseAPI ocr;
    if (ocr.Init(nullptr, "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return;
    }

    // Convert the OpenCV image to a format Tesseract can understand
    ocr.SetImage(image.data, image.cols, image.rows, 4, image.step);
    std::string text = std::string(ocr.GetUTF8Text());

    // Save the extracted text to a file
    std::ofstream out(output_text_file);
    out << text;
    out.close();

    ocr.End();
}

int main() {
    cv::Mat img = cv::imread("example_image.png");
    if (img.empty()) {
        std::cerr << "Error loading image\n";
        return 1;
    }

    performOCR(img, "output_text.txt");

    return 0;
}
