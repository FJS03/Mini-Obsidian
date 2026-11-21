#include "../include/PdfExport.h"
#include "../include/PreviewEngine.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

bool PdfExport::exportNoteToPdf(const std::string& noteTitle, const std::string& markdownText, const std::string& outputPdfPath) {
    // 1. Convert Markdown to HTML
    std::string bodyHtml = PreviewEngine::markdownToHtml(markdownText);

    std::string printHtml = 
        "<!DOCTYPE html>\n<html>\n<head>\n"
        "<meta charset='utf-8'>\n"
        "<title>" + noteTitle + "</title>\n"
        "<style>\n"
        "  body { font-family: 'Segoe UI', Arial, sans-serif; color: #111; max-width: 800px; margin: 40px auto; padding: 20px; line-height: 1.6; }\n"
        "  h1 { color: #000; border-bottom: 2px solid #ccc; padding-bottom: 8px; }\n"
        "  h2, h3 { color: #222; margin-top: 20px; }\n"
        "  p { font-size: 15px; }\n"
        "  ul { padding-left: 25px; }\n"
        "  li { margin-bottom: 5px; }\n"
        "</style>\n"
        "</head>\n<body>\n"
        "<h1>" + noteTitle + "</h1>\n"
        + bodyHtml +
        "\n</body>\n</html>";

    std::string tempHtml = "temp_print.html";
    std::ofstream htmlFile(tempHtml);
    if (!htmlFile.is_open()) return false;
    htmlFile << printHtml;
    htmlFile.close();

    // 2. Find browser executable path
    std::string browserPath = "msedge";
    if (fs::exists("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe")) {
        browserPath = "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";
    } else if (fs::exists("C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe")) {
        browserPath = "C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe";
    } else if (fs::exists("C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe")) {
        browserPath = "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
    }

    std::string absHtml = fs::absolute(tempHtml).string();
    std::string absPdf = fs::absolute(outputPdfPath).string();

    // 3. Write a tiny helper batch file to run Edge/Chrome without quoting bugs
    std::ofstream batFile("do_pdf.bat");
    if (batFile.is_open()) {
        batFile << "@echo off\n";
        batFile << "\"" << browserPath << "\" --headless --disable-gpu --run-all-compositor-stages-before-draw --print-to-pdf=\"" << absPdf << "\" \"" << absHtml << "\"\n";
        batFile.close();
    }

    // Run the batch file
    system("do_pdf.bat >nul 2>&1");

    // Clean up temporary files
    fs::remove("do_pdf.bat");
    fs::remove(tempHtml);

    return fs::exists(absPdf);
}