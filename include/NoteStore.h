#pragma once
#include <string>
#include <vector>

struct Note {
    int id;
    std::string title;
    std::string content;
    std::string filePath;
    std::string createdAt;
};

class NoteStore {
public:
    // Initializes the SQLite database & creates tables if they don't exist
    static bool initDatabase(const std::string& dbPath = "notes.db");

    // Create a new note, saves to disk & inserts into SQLite
    static int createNote(const std::string& title, const std::string& content);

    // Get a specific note by ID
    static Note getNote(int id);

    // List all existing notes from database
    static std::vector<Note> listAllNotes();

    // Instant search across titles and note bodies
    static std::vector<Note> searchNotes(const std::string& query);

    // Delete a note by ID
    static bool deleteNote(int id);

    // Closes the database connection
    static void closeDatabase();
};