#include "../include/CLI.h"
#include "../include/NoteStore.h"
#include "../include/PreviewEngine.h"
#include "../include/PdfExport.h"
#include <iostream>
#include <string>
#include <limits>

void CLI::printHeader() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "         MINI-OBSIDIAN (CLI)           " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << " 1. List all notes" << std::endl;
    std::cout << " 2. Create a new note" << std::endl;
    std::cout << " 3. View / Open a note" << std::endl;
    std::cout << " 4. Search notes (FTS5)" << std::endl;
    std::cout << " 5. Delete a note" << std::endl;
    std::cout << " 6. Export note to PDF" << std::endl;
    std::cout << " 7. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Choose an option (1-7): ";
}

void CLI::run() {
    int choice = 0;
    while (true) {
        printHeader();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\n[!] Invalid input. Please enter a number." << std::endl;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            handleList();
        } else if (choice == 2) {
            handleCreate();
        } else if (choice == 3) {
            handleView();
        } else if (choice == 4) {
            handleSearch();
        } else if (choice == 5) {
            handleDelete();
        } else if (choice == 6) {
            handlePdfExport();
        } else if (choice == 7) {
            std::cout << "\nGoodbye! Thank you for using Mini-Obsidian.\n" << std::endl;
            break;
        } else {
            std::cout << "\n[!] Unknown option. Choose 1 to 7." << std::endl;
        }
    }
}

void CLI::handleList() {
    std::cout << "\n--- All Notes ---" << std::endl;
    std::vector<Note> notes = NoteStore::listAllNotes();
    if (notes.empty()) {
        std::cout << "No notes found. Create your first note!" << std::endl;
        return;
    }
    for (const auto& n : notes) {
        std::cout << "[" << n.id << "] " << n.title 
                  << "  (created: " << n.createdAt << ")" << std::endl;
    }
}

void CLI::handleCreate() {
    std::cout << "\n--- Create Note ---" << std::endl;
    std::cout << "Enter Note Title: ";
    std::string title;
    std::getline(std::cin, title);

    if (title.empty()) {
        std::cout << "[!] Title cannot be empty." << std::endl;
        return;
    }

    std::cout << "Enter Note Content (Markdown supported):\n> ";
    std::string content;
    std::getline(std::cin, content);

    int id = NoteStore::createNote(title, content);
    std::cout << "\n[SUCCESS] Note saved with ID: " << id << std::endl;
}

void CLI::handleView() {
    std::cout << "\n--- View Note ---" << std::endl;
    std::cout << "Enter Note ID to open: ";
    int id;
    if (!(std::cin >> id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Invalid ID." << std::endl;
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Note note = NoteStore::getNote(id);
    if (note.id == -1) {
        std::cout << "[!] Note with ID " << id << " not found." << std::endl;
        return;
    }

    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "Title:   " << note.title << std::endl;
    std::cout << "File:    notes/" << note.filePath << std::endl;
    std::cout << "Created: " << note.createdAt << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << note.content << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    std::cout << "\nOpen formatted preview in browser? (y/n): ";
    char ans;
    std::cin >> ans;
    if (ans == 'y' || ans == 'Y') {
        PreviewEngine::openPreviewInBrowser(note.title, note.content);
        std::cout << "[SUCCESS] Opened preview.html in browser!" << std::endl;
    }
}

void CLI::handleSearch() {
    std::cout << "\n--- Instant Search (FTS5) ---" << std::endl;
    std::cout << "Enter search keyword: ";
    std::string query;
    std::getline(std::cin, query);

    std::vector<Note> results = NoteStore::searchNotes(query);
    if (results.empty()) {
        std::cout << "No matching notes found for '" << query << "'." << std::endl;
        return;
    }

    std::cout << "\nFound " << results.size() << " match(es):" << std::endl;
    for (const auto& r : results) {
        std::cout << "\n-> [" << r.id << "] " << r.title << std::endl;
        std::cout << "   Snippet: " << r.content << std::endl;
    }
}

void CLI::handleDelete() {
    std::cout << "\n--- Delete Note ---" << std::endl;
    std::cout << "Enter Note ID to delete: ";
    int id;
    if (!(std::cin >> id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Invalid ID." << std::endl;
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (NoteStore::deleteNote(id)) {
        std::cout << "[SUCCESS] Note ID " << id << " and its .md file were deleted." << std::endl;
    } else {
        std::cout << "[!] Note not found." << std::endl;
    }
}

void CLI::handlePdfExport() {
    std::cout << "\n--- Export Note to PDF ---" << std::endl;
    std::cout << "Enter Note ID to export: ";
    int id;
    if (!(std::cin >> id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "[!] Invalid ID." << std::endl;
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Note note = NoteStore::getNote(id);
    if (note.id == -1) {
        std::cout << "[!] Note with ID " << id << " not found." << std::endl;
        return;
    }

    // A note titled "Q: ideas?" must not become an illegal Windows filename.
    std::string pdfName = note.title;
    for (char& c : pdfName) {
        if ((unsigned char)c < 32 || std::string("<>:\"/\\|?*").find(c) != std::string::npos) c = '_';
    }
    if (pdfName.empty()) pdfName = "note";

    std::string pdfFilename = pdfName + ".pdf";
    std::cout << "Exporting to " << pdfFilename << "..." << std::endl;

    if (PdfExport::exportNoteToPdf(note.title, note.content, pdfFilename)) {
        std::cout << "[SUCCESS] PDF generated successfully: " << pdfFilename << std::endl;
    } else {
        std::cout << "[!] Failed to export PDF." << std::endl;
    }
}