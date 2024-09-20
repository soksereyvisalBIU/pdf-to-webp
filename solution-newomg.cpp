#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

/**
 * Convert a Poppler image to OpenCV's cv::Mat format.
 *
 * @param img The Poppler image to convert.
 * @return cv::Mat The converted OpenCV image.
 */
cv::Mat popplerImageToMat(poppler::image& img)
{
    int width = img.width();
    int height = img.height();
    int bytes_per_row = img.bytes_per_row();

    // Create a cv::Mat from the image data, then clone to ensure memory safety.
    return cv::Mat(height, width, CV_8UC4, img.data(), bytes_per_row).clone();
}

/**
 * Resize the image to a specified width, maintaining aspect ratio.
 *
 * @param img The input image.
 * @param target_width The desired width after resizing.
 * @return cv::Mat The resized image.
 */
cv::Mat resizeImage(const cv::Mat& img, int target_width)
{
    int original_width = img.cols;
    int original_height = img.rows;
    int target_height = (target_width * original_height) / original_width;

    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(target_width, target_height));

    return resized_img;
}

/**
 * Perform OCR (Optical Character Recognition) on an image and save the recognized text to a file.
 *
 * @param image The input image for OCR.
 * @param output_text_file The file path to save the recognized text.
 */
void performOCR(const cv::Mat& image, const string& output_text_file)
{
    tesseract::TessBaseAPI ocr;
    if (ocr.Init(nullptr, "eng"))
    {
        cerr << "Could not initialize tesseract.\n";
        return;
    }

    // Set the image for Tesseract to perform OCR on.
    ocr.SetImage(image.data, image.cols, image.rows, 4, image.step);
    string text = string(ocr.GetUTF8Text());

    // Save the recognized text to a file.
    ofstream out(output_text_file);
    out << text;
    out.close();

    ocr.End();
}

/**
 * Save an image as a WebP file with a specified quality.
 *
 * @param image The input image to save.
 * @param output_file The file path to save the WebP image.
 * @param quality The quality of the saved WebP image (0-100).
 */
void saveAsWebP(const cv::Mat& image, const string& output_file, int quality)
{
    vector<int> compression_params = { cv::IMWRITE_WEBP_QUALITY, quality };
    if (!cv::imwrite(output_file, image, compression_params))
    {
        cerr << "Error saving image as WebP.\n";
    }
}

/**
 * Process a single PDF file: render each page, resize, convert to WebP, and perform OCR.
 *
 * @param pdf_file The path to the input PDF file.
 * @param output_dir The directory where the output images and text files will be saved.
 * @param dpi The DPI (dots per inch) for rendering the PDF pages.
 * @param target_width The desired width for the output images.
 * @param quality The quality for the output WebP images.
 */
void processPDF(const string& pdf_file, const string& output_dir, int dpi, int target_width, int quality)
{
    // Load the PDF document.
    unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_file));
    if (!doc)
    {
        cerr << "Error loading PDF file: " << pdf_file << endl;
        return;
    }

    // Create the output directory if it doesn't exist.
    fs::create_directories(output_dir);

    // Iterate through each page in the PDF.
    for (int i = 0; i < doc->pages(); ++i)
    {
        unique_ptr<poppler::page> p(doc->create_page(i));
        if (p)
        {
            // Render the page to an image.
            poppler::page_renderer renderer;
            renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
            renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

            poppler::image img = renderer.render_page(p.get(), dpi, dpi);
            cv::Mat mat_img = popplerImageToMat(img);

            // Resize the image.
            cv::Mat resized_img = resizeImage(mat_img, target_width);

            // Save the image as a WebP file.
            string page_file = output_dir + "/p_" + to_string(i + 1) + ".webp";
            saveAsWebP(resized_img, page_file, quality);

            // Perform OCR on the image and save the text.
            string text_file = output_dir + "/p_" + to_string(i + 1) + ".txt";
            performOCR(resized_img, text_file);
        }
    }

    // // Optionally, create an HTML file to display the images.
    // string html_file = output_dir + "/index.html";
    // ofstream html_out(html_file);
    // html_out << "<!DOCTYPE html>\n<html>\n<body>\n";
    // for (int i = 0; i < doc->pages(); ++i)
    // {
    //     html_out << "<img src=\"p_" << i << ".webp\" alt=\"Page " << i + 1 << "\"><br>\n";
    // }
    // html_out << "</body>\n</html>\n";
    // html_out.close();
}

/**
 * Recursively process all PDF files in a directory.
 *
 * @param input_dir The directory containing the PDF files.
 * @param output_dir The directory where the output files will be saved.
 * @param dpi The DPI for rendering the PDF pages.
 * @param target_width The desired width for the output images.
 * @param quality The quality for the output WebP images.
 */
void processDirectory(const string& input_dir, const string& output_dir, int dpi, int target_width, int quality)
{
    // Iterate through all files and directories in the input directory
    for (const auto& entry : fs::recursive_directory_iterator(input_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf")
        {
            // Get the relative path of the file within the input directory
            string relative_path = fs::relative(entry.path(), input_dir).string();

            // Generate the output directory path based on the relative path
            fs::path output_pdf_path = fs::path(output_dir) / fs::path(relative_path).parent_path();

            // Create a subdirectory named after the PDF file (excluding the extension)
            string base_name = entry.path().stem().string();
            fs::path pdf_output_dir = output_pdf_path / base_name;
            fs::create_directories(pdf_output_dir);

            // Process the PDF file
            processPDF(entry.path().string(), pdf_output_dir.string(), dpi, target_width, quality);
        }
    }
}

int main()
{
    // Define input/output directories, DPI, target image width, and WebP quality.
    string input_dir = "D:/omg/pdf-newomg";
    string output_dir = "D:/omg/pdfs";
    int dpi = 150;
    int target_width = 700;
    int quality = 75;

    // Process the input directory.
    processDirectory(input_dir, output_dir, dpi, target_width, quality);

    return 0;
}
