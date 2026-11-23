#include "../include/PdfExport.h"
#include "../include/PreviewEngine.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include <windows.h>

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
        "  strong { font-weight: 700; color: #000; }\n"
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

    // 2. Search for Microsoft Edge or Chrome in all standard Windows paths
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

    // Delete any old pdf if it exists
    if (fs::exists(absPdf)) {
        fs::remove(absPdf);
    }

    // 3. Inheritable NUL handle setup (silences 100% of Chromium console noise)
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hNul = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = hNul;
    si.hStdOutput = hNul;
    si.hStdError = hNul;
    ZeroMemory(&pi, sizeof(pi));

    std::string cmdArgs = "\"" + browserExe + "\" --headless --disable-gpu --no-sandbox --no-pdf-header-footer --log-level=3 --silent --disable-logging --print-to-pdf=\"" + absPdf + "\" \"" + absHtml + "\"";
    std::vector<char> cmdVec(cmdArgs.begin(), cmdArgs.end());
    cmdVec.push_back('\0');

    if (CreateProcessA(NULL, cmdVec.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 10000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    if (hNul != INVALID_HANDLE_VALUE) {
        CloseHandle(hNul);
    }

    // Wait up to 1 second for Windows file flush
    for (int i = 0; i < 5; ++i) {
        if (fs::exists(absPdf) && fs::file_size(absPdf) > 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Cleanup temporary HTML file
    fs::remove(tempHtml);

    return fs::exists(absPdf) && fs::file_size(absPdf) > 0;
}
