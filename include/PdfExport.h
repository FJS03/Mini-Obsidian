#pragma once
#include <string>

class PdfExport {
public:
    // Exports a note to a PDF file. Returns true if created successfully.
    static bool exportNoteToPdf(const std::string& noteTitle, const std::string& markdownText, const std::string& outputPdfPath);
};