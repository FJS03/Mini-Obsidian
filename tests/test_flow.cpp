#include <iostream>
#include <cassert>
#include <filesystem>
#include "../include/NoteStore.h"
#include "../include/PdfExport.h"

namespace fs = std::filesystem;

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  RUNNING MINI-OBSIDIAN TEST SUITE      " << std::endl;
    std::cout << "========================================" << std::endl;

    // 1. Initialize Test Database
    bool dbOk = NoteStore::initDatabase("test_notes.db");
    assert(dbOk && "Database initialization failed");
    std::cout << "[PASS] 1. Database initialized successfully" << std::endl;

    // 2. Test CREATE
    int id = NoteStore::createNote("Algorithms", "# Sorting\nQuickSort runs in O(N log N) average time.");
    assert(id > 0 && "Note creation failed");
    std::cout << "[PASS] 2. Create Note (Generated ID: " << id << ")" << std::endl;

    // 3. Test READ
    Note n = NoteStore::getNote(id);
    assert(n.title == "Algorithms" && "Title mismatch");
    assert(!n.content.empty() && "Content is empty");
    std::cout << "[PASS] 3. Read Note from disk & database" << std::endl;

    // 4. Test FTS5 SEARCH
    auto searchResults = NoteStore::searchNotes("QuickSort");
    assert(!searchResults.empty() && "Search failed to find keyword");
    assert(searchResults[0].id == id && "Search returned wrong note");
    std::cout << "[PASS] 4. FTS5 Full-Text Search ('QuickSort' found instantly)" << std::endl;

    // 5. Test PDF EXPORT
    std::string testPdf = "test_output.pdf";
    bool pdfOk = PdfExport::exportNoteToPdf(n.title, n.content, testPdf);
    assert(pdfOk && "PDF export failed");
    std::cout << "[PASS] 5. Export Note to PDF ('test_output.pdf' generated)" << std::endl;

    // 6. Test DELETE
    bool delOk = NoteStore::deleteNote(id);
    assert(delOk && "Delete failed");
    Note deletedNote = NoteStore::getNote(id);
    assert(deletedNote.id == -1 && "Note still exists after deletion");
    std::cout << "[PASS] 6. Delete Note & cleanup verified" << std::endl;

    NoteStore::closeDatabase();

    // Clean up test files
    fs::remove("test_notes.db");
    fs::remove(testPdf);

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL 6 TESTS PASSED! (100% SUCCESS)    " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}