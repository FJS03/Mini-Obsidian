#pragma once
#include <string>
#include <vector>

class FileIO {
public:
    // Saves text into a file. Returns true if successful.
    static bool saveNote(const std::string& filename, const std::string& content);

    // Reads all text from a file.
    static std::string readNote(const std::string& filename);

    // Deletes a file. Returns true if deleted.
    static bool deleteNote(const std::string& filename);
};
