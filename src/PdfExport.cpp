#include "../include/PdfExport.h"
#include "../include/PreviewEngine.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <windows.h>

namespace fs = std::filesystem;

static bool fileHasData(const std::string& path) {
    std::error_code ec;
    auto size = fs::file_size(path, ec);
    return !ec && size > 0;
}

// Windows only accepts "C:/dir/file.html" as a URL, never "C:\dir\file.html".
static std::string toFileUrl(const std::string& windowsPath) {
    std::string url = windowsPath;
    std::replace(url.begin(), url.end(), '\\', '/');
    if (url.size() > 1 && url[1] == ':') {
        url = "file:///" + url;
    }
    return url;
}

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

    // Scratch files go to the system temp folder so the working directory does not matter.
    std::string stamp = std::to_string(GetCurrentProcessId());
    std::string absHtml = (fs::temp_directory_path() / ("mini_obsidian_" + stamp + ".html")).string();
    std::string profileDir = (fs::temp_directory_path() / ("mini_obsidian_profile_" + stamp)).string();
    std::string absPdf = fs::absolute(outputPdfPath).string();

    std::ofstream htmlFile(absHtml);
    if (!htmlFile.is_open()) {
        std::cerr << "[PDF] Cannot write temporary HTML file: " << absHtml << std::endl;
        return false;
    }
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

    char* localApp = std::getenv("LOCALAPPDATA");
    if (localApp) {
        candidates.push_back(std::string(localApp) + "\\Microsoft\\Edge\\Application\\msedge.exe");
        candidates.push_back(std::string(localApp) + "\\Google\\Chrome\\Application\\chrome.exe");
    }

    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            browserExe = path;
            break;
        }
    }

    if (browserExe.empty()) {
        char searchBuf[MAX_PATH];
        if (SearchPathA(NULL, "msedge.exe", NULL, MAX_PATH, searchBuf, NULL)) {
            browserExe = searchBuf;
        } else if (SearchPathA(NULL, "chrome.exe", NULL, MAX_PATH, searchBuf, NULL)) {
            browserExe = searchBuf;
        }
    }

    if (browserExe.empty()) {
        std::cerr << "[PDF] No Edge or Chrome found. Install one, or add it to PATH." << std::endl;
        fs::remove(absHtml);
        return false;
    }

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

    // A dedicated --user-data-dir means a browser the user already has open cannot
    // swallow this headless run, which would otherwise exit without writing the PDF.
    std::string cmdArgs =
        "\"" + browserExe + "\" --headless --disable-gpu --no-sandbox --no-first-run"
        " --no-default-browser-check --no-pdf-header-footer --log-level=3 --silent --disable-logging"
        " --user-data-dir=\"" + profileDir + "\""
        " --print-to-pdf=\"" + absPdf + "\" \"" + toFileUrl(absHtml) + "\"";
    std::vector<char> cmdVec(cmdArgs.begin(), cmdArgs.end());
    cmdVec.push_back('\0');

    bool started = CreateProcessA(browserExe.c_str(), cmdVec.data(), NULL, NULL, TRUE,
                                  CREATE_NO_WINDOW, NULL, NULL, &si, &pi) != 0;
    if (started) {
        WaitForSingleObject(pi.hProcess, 20000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        std::cerr << "[PDF] Cannot start browser: " << browserExe
                  << " (error " << GetLastError() << ")" << std::endl;
    }

    if (hNul != INVALID_HANDLE_VALUE) {
        CloseHandle(hNul);
    }

    // 4. Give Windows a moment to flush the file to disk
    for (int i = 0; i < 25 && !fileHasData(absPdf); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Cleanup scratch files
    fs::remove(absHtml);
    std::error_code ec;
    fs::remove_all(profileDir, ec);

    if (!fileHasData(absPdf)) {
        std::cerr << "[PDF] Browser did not write: " << absPdf << std::endl;
        return false;
    }
    return true;
}