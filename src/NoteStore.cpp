#include "../include/NoteStore.h"
#include "../include/FileIO.h"
#include "../include/sqlite3.h"
#include <iostream>
#include <ctime>
#include <algorithm>

static sqlite3* g_db = nullptr;

static std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

bool NoteStore::initDatabase(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &g_db) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database: " << sqlite3_errmsg(g_db) << std::endl;
        return false;
    }

    // 1. Create the main notes table
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS notes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "filepath TEXT NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(g_db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 2. Create the FTS5 Full-Text Search virtual table
    const char* createFtsSQL = 
        "CREATE VIRTUAL TABLE IF NOT EXISTS notes_fts USING fts5("
        "title, content, content_rowid='id'"
        ");";
    sqlite3_exec(g_db, createFtsSQL, nullptr, nullptr, nullptr);

    return true;
}

int NoteStore::createNote(const std::string& title, const std::string& content) {
    if (!g_db) return -1;

    // 1. Generate safe filename
    std::string safeName = title;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    std::string filePath = safeName + ".md";
    std::string createdAt = getCurrentTime();

    // 2. Save .md file to disk
    FileIO::saveNote(filePath, content);

    // 3. Insert into SQLite table
    const char* sql = "INSERT INTO notes (title, filepath, created_at) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, filePath.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, createdAt.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    int noteId = (int)sqlite3_last_insert_rowid(g_db);

    // 4. Insert into FTS5 index for instant search
    const char* ftsSql = "INSERT INTO notes_fts (rowid, title, content) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(g_db, ftsSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, noteId);
        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    return noteId;
}

Note NoteStore::getNote(int id) {
    Note note{-1, "", "", "", ""};
    if (!g_db) return note;

    const char* sql = "SELECT id, title, filepath, created_at FROM notes WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            note.id = sqlite3_column_int(stmt, 0);
            note.title = (const char*)sqlite3_column_text(stmt, 1);
            note.filePath = (const char*)sqlite3_column_text(stmt, 2);
            note.createdAt = (const char*)sqlite3_column_text(stmt, 3);
            note.content = FileIO::readNote(note.filePath);
        }
        sqlite3_finalize(stmt);
    }
    return note;
}

std::vector<Note> NoteStore::listAllNotes() {
    std::vector<Note> list;
    if (!g_db) return list;

    const char* sql = "SELECT id, title, filepath, created_at FROM notes ORDER BY id DESC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Note n;
            n.id = sqlite3_column_int(stmt, 0);
            n.title = (const char*)sqlite3_column_text(stmt, 1);
            n.filePath = (const char*)sqlite3_column_text(stmt, 2);
            n.createdAt = (const char*)sqlite3_column_text(stmt, 3);
            list.push_back(n);
        }
        sqlite3_finalize(stmt);
    }
    return list;
}

std::vector<Note> NoteStore::searchNotes(const std::string& query) {
    std::vector<Note> results;
    if (!g_db) return results;

    // FTS5 MATCH query: searches both title and markdown body in milliseconds
    const char* sql = 
        "SELECT n.id, n.title, n.filepath, n.created_at "
        "FROM notes n "
        "JOIN notes_fts ON n.id = notes_fts.rowid "
        "WHERE notes_fts MATCH ? "
        "ORDER BY rank;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Note n;
            n.id = sqlite3_column_int(stmt, 0);
            n.title = (const char*)sqlite3_column_text(stmt, 1);
            n.filePath = (const char*)sqlite3_column_text(stmt, 2);
            n.createdAt = (const char*)sqlite3_column_text(stmt, 3);
            n.content = FileIO::readNote(n.filePath);
            results.push_back(n);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

bool NoteStore::deleteNote(int id) {
    if (!g_db) return false;

    // 1. Get filepath to delete file from disk
    Note note = getNote(id);
    if (note.id != -1) {
        FileIO::deleteNote(note.filePath);
    }

    // 2. Remove from SQLite tables
    const char* sql = "DELETE FROM notes WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* ftsSql = "DELETE FROM notes_fts WHERE rowid = ?;";
    if (sqlite3_prepare_v2(g_db, ftsSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    return true;
}

void NoteStore::closeDatabase() {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = nullptr;
    }
}