#pragma once
#include <string>

class PreviewEngine {
public:
    // Converts raw Markdown text into full HTML
    static std::string markdownToHtml(const std::string& markdownText);

    // Generates a preview.html file and opens it in the default browser
    static bool openPreviewInBrowser(const std::string& noteTitle, const std::string& markdownText);
};