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

cv::Mat popplerImageToMat(poppler::image &img)
{
    int width = img.width();
    int height = img.height();
    int bytes_per_row = img.bytes_per_row();
    return cv::Mat(height, width, CV_8UC4, img.data(), bytes_per_row).clone();
}

cv::Mat resize_image(const cv::Mat &img, int target_width)
{
    int original_width = img.cols;
    int original_height = img.rows;
    int target_height = (target_width * original_height) / original_width;

    cv::Mat resized_img;
    cv::resize(img, resized_img, cv::Size(target_width, target_height));

    return resized_img;
}

string perform_ocr(const cv::Mat &image, tesseract::TessBaseAPI &ocr)
{
    ocr.SetImage(image.data, image.cols, image.rows, 4, image.step);
    char *outText = ocr.GetUTF8Text();
    string text(outText);
    delete[] outText;
    return text;
}

void process_pdf(const string &input_file, const string &output_folder, int dpi, int target_width, tesseract::TessBaseAPI &ocr)
{
    unique_ptr<poppler::document> doc(poppler::document::load_from_file(input_file));
    if (!doc)
    {
        cerr << "Error loading PDF file: " << input_file << endl;
        return;
    }

    string pdf_name = fs::path(input_file).stem().string();
    string output_pdf_folder = output_folder + "/" + pdf_name;
    fs::create_directories(output_pdf_folder);

    for (int i = 0; i < doc->pages(); ++i)
    {
        unique_ptr<poppler::page> p(doc->create_page(i));
        if (p)
        {
            poppler::page_renderer renderer;
            renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
            renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

            poppler::image img = renderer.render_page(p.get(), dpi, dpi);
            cv::Mat mat_img = popplerImageToMat(img);
            cv::Mat resized_mat_img = resize_image(mat_img, target_width);

            // Perform OCR on the resized image
            string ocr_text = perform_ocr(resized_mat_img, ocr);
            cout << "OCR Text for page " << i << ": " << ocr_text << endl;

            string page_file = output_pdf_folder + "/p_" + to_string(i) + ".webp";
            cv::imwrite(page_file, resized_mat_img, {cv::IMWRITE_WEBP_QUALITY, 80});
        }
    }
}

void resize_pdfs_in_directory(const string &input_folder, const string &output_folder, int dpi, int target_width)
{
    for (const auto &entry : fs::recursive_directory_iterator(input_folder))
    {
        if (entry.path().extension() == ".pdf")
        {
            cout << "Processing PDF: " << entry.path().string() << endl;

            tesseract::TessBaseAPI ocr;
            if (ocr.Init(NULL, "eng"))
            {
                cerr << "Could not initialize tesseract for: " << entry.path().string() << endl;
                continue;
            }

            process_pdf(entry.path().string(), output_folder, dpi, target_width, ocr);

            ocr.End();
        }
    }
}

int main()
{
    // Set TESSDATA_PREFIX environment variable using _putenv on Windows
    _putenv("TESSDATA_PREFIX=C:/msys64/mingw64/share/");

    string input_folder = "C:/Users/Pulse/Desktop/cpp/pdffolder";
    string output_folder = "C:/Users/Pulse/Desktop/cpp/hehehnotbad";

    int dpi = 150;
    int target_width = 800;

    resize_pdfs_in_directory(input_folder, output_folder, dpi, target_width);

    return 0;
}
