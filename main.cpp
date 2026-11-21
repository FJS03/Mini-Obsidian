#include <iostream>
#include "include/NoteStore.h"
#include "include/CLI.h"

int main() {
    // 1. Initialize Database
    if (!NoteStore::initDatabase("notes.db")) {
        std::cerr << "Failed to initialize database." << std::endl;
        return 1;
    }

    // 2. Launch interactive UI
    CLI::run();

    // 3. Clean up before closing
    NoteStore::closeDatabase();
    return 0;
}