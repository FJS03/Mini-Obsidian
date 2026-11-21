#include "../include/FileIO.h"
#include <fstream>
#include <sstream>
#include <cstdio>

bool FileIO::saveNote(const std::string& filename, const std::string& content) {
    std::string fullPath = "notes/" + filename;
    std::ofstream outFile(fullPath);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << content;
    outFile.close();
    return true;
}

std::string FileIO::readNote(const std::string& filename) {
    std::string fullPath = "notes/" + filename;
    std::ifstream inFile(fullPath);
    if (!inFile.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    return buffer.str();
}

bool FileIO::deleteNote(const std::string& filename) {
    std::string fullPath = "notes/" + filename;
    return (std::remove(fullPath.c_str()) == 0);
}