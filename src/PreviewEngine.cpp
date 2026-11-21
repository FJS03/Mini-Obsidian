#include "../include/PreviewEngine.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>

std::string PreviewEngine::markdownToHtml(const std::string& markdownText) {
    std::stringstream in(markdownText);
    std::stringstream out;
    std::string line;
    bool inList = false;

    while (std::getline(in, line)) {
        if (line.empty()) {
            if (inList) {
                out << "</ul>\n";
                inList = false;
            }
            continue;
        }

        // 1. Headings (# Heading, ## Heading)
        if (line.rfind("### ", 0) == 0) {
            out << "<h3>" << line.substr(4) << "</h3>\n";
            continue;
        }
        if (line.rfind("## ", 0) == 0) {
            out << "<h2>" << line.substr(3) << "</h2>\n";
            continue;
        }
        if (line.rfind("# ", 0) == 0) {
            out << "<h1>" << line.substr(2) << "</h1>\n";
            continue;
        }

        // 2. Unordered lists (- Item or * Item)
        if (line.rfind("- ", 0) == 0 || line.rfind("* ", 0) == 0) {
            if (!inList) {
                out << "<ul>\n";
                inList = true;
            }
            out << "  <li>" << line.substr(2) << "</li>\n";
            continue;
        } else if (inList) {
            out << "</ul>\n";
            inList = false;
        }

        // 3. Inline Bold (**text**) and Italic (*text*)
        std::string parsedLine = std::regex_replace(line, std::regex("\\*\\*(.*?)\\*\\*"), "<strong>$1</strong>");
        parsedLine = std::regex_replace(parsedLine, std::regex("\\*(.*?)\\*"), "<em>$1</em>");

        // 4. Regular paragraph
        out << "<p>" << parsedLine << "</p>\n";
    }

    if (inList) {
        out << "</ul>\n";
    }

    return out.str();
}

bool PreviewEngine::openPreviewInBrowser(const std::string& noteTitle, const std::string& markdownText) {
    std::string bodyHtml = markdownToHtml(markdownText);

    // Wrap in a sleek, modern Obsidian-style dark theme template
    std::string fullHtml = 
        "<!DOCTYPE html>\n<html>\n<head>\n"
        "<meta charset='utf-8'>\n"
        "<title>" + noteTitle + " - Preview</title>\n"
        "<style>\n"
        "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif; "
        "         background-color: #1e1e1e; color: #dcddde; max-width: 800px; margin: 40px auto; padding: 20px; line-height: 1.6; }\n"
        "  h1, h2, h3 { color: #8a63d2; border-bottom: 1px solid #333; padding-bottom: 8px; }\n"
        "  p { font-size: 16px; }\n"
        "  ul { padding-left: 25px; }\n"
        "  li { margin-bottom: 6px; }\n"
        "  strong { color: #e0af68; }\n"
        "  em { color: #7aa2f7; }\n"
        "</style>\n"
        "</head>\n<body>\n" + bodyHtml + "</body>\n</html>";

    std::ofstream file("preview.html");
    if (!file.is_open()) return false;
    file << fullHtml;
    file.close();

    // Open automatically in default browser (Windows)
    system("start preview.html");
    return true;
}