#include "../include/PdfExport.h"
#include "../include/PreviewEngine.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

bool PdfExport::exportNoteToPdf(const std::string& noteTitle, const std::string& markdownText, const std::string& outputPdfPath) {
    // 1. Convert Markdown to clean HTML
    std::string bodyHtml = PreviewEngine::markdownToHtml(markdownText);

    std::string printHtml = 
        "<!DOCTYPE html>\n<html>\n<head>\n"
        "<meta charset='utf-8'>\n"
        "<title>" + noteTitle + "</title>\n"
        "<style>\n"
        "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; "
        "         color: #111; max-width: 800px; margin: 40px auto; padding: 20px; line-height: 1.6; }\n"
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

    // 2. Locate browser executable
    std::string browserExe = "";
    std::vector<std::string> candidates = {
        "C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
        "C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe",
        "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
        "C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe"
    };

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            browserExe = path;
            break;
        }
    }

    if (browserExe.empty()) {
        browserExe = "msedge.exe";
    }

    std::string absHtml = fs::absolute(tempHtml).string();
    std::string absPdf = fs::absolute(outputPdfPath).string();

    // Remove old PDF if it exists
    if (fs::exists(absPdf)) {
        fs::remove(absPdf);
    }

    // 3. PowerShell call: silences graphics logs with --log-level=3 and runs cleanly
    std::string psCmd = 
        "powershell -NoProfile -ExecutionPolicy Bypass -Command \""
        "Start-Process -FilePath '" + browserExe + "' "
        "-ArgumentList '--headless', '--disable-gpu', '--no-sandbox', '--no-pdf-header-footer', "
        "'--log-level=3', "
        "'--print-to-pdf=\\\"" + absPdf + "\\\"', '\\\"" + absHtml + "\\\"' -Wait\" >nul 2>&1";

    system(psCmd.c_str());

    // Wait briefly for file write
    for (int i = 0; i < 5; ++i) {
        if (fs::exists(absPdf) && fs::file_size(absPdf) > 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Clean up temporary HTML
    fs::remove(tempHtml);

    return fs::exists(absPdf) && fs::file_size(absPdf) > 0;
}