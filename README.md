# Mini-Obsidian

> A Lightweight Markdown Note-Taking System with Instant Full-Text Search and PDF Export, written in modern C++.

Mini-Obsidian is a fast, local-first note application built to solve the memory-heavy problems of Electron-based apps (like Obsidian, Notion, and Joplin). It stores notes as plain Markdown files on disk, indexes them with SQLite FTS5 for instant search, renders a dark-mode preview in your browser, and exports notes directly to PDF.

---

## Key Features

- **Local-First & Private:** Notes are saved as plain `.md` files in a local folder. You own your files—no cloud, no accounts, no internet required.
- **Instant Full-Text Search (SQLite FTS5):** Search across all note titles and bodies in milliseconds using SQLite's high-speed FTS5 index.
- **Obsidian Dark Preview:** Converts Markdown into clean HTML with an Obsidian-inspired dark theme and launches it directly in your browser.
- **One-Step PDF Export:** Automatically generates print-ready PDF files for homework, assignments, and reports.
- **Interactive CLI:** Fast, responsive terminal interface to create, list, view, search, and delete notes.
- **Automated QA:** Includes an automated integration test suite verifying the complete `create -> read -> search -> export -> delete` lifecycle.

---

## Architecture & Project Structure

The project follows a clean, modular architecture separating the storage, search, preview, and user interface layers:

```
Mini-Obsidian/
├── include/              # Header contracts (API blueprints)
│   ├── FileIO.h          # Raw disk file operations (.md files)
│   ├── NoteStore.h       # Core Note model & SQLite manager
│   ├── PreviewEngine.h   # Markdown parser & HTML generator
│   ├── PdfExport.h       # Headless PDF export engine
│   ├── CLI.h             # Interactive terminal interface
│   └── sqlite3.h         # Embedded database engine header
├── src/                  # Implementation files
│   ├── FileIO.cpp
│   ├── NoteStore.cpp
│   ├── PreviewEngine.cpp
│   ├── PdfExport.cpp
│   ├── CLI.cpp
│   └── sqlite3.c
├── tests/                # Automated test suite
│   └── test_flow.cpp
├── notes/                # Storage directory for user .md files
├── CMakeLists.txt        # Cross-platform build recipe
└── README.md
```

---

## Building & Running

### Prerequisites
- Modern C++ Compiler (GCC / MinGW supporting C++17)
- CMake 3.15 or newer

### 1. Build the Application
```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

### 2. Run Mini-Obsidian
```bash
./build/mini_obsidian.exe
```

### 3. Run Automated Tests
```bash
./build/run_tests.exe
```

---

## Test Results

The automated integration test suite verifies the system across 6 critical checkpoints:

```text
========================================
  RUNNING MINI-OBSIDIAN TEST SUITE      
========================================
[PASS] 1. Database initialized successfully
[PASS] 2. Create Note (Generated ID: 1)
[PASS] 3. Read Note from disk & database
[PASS] 4. FTS5 Full-Text Search ('QuickSort' found instantly)
[PASS] 5. Export Note to PDF ('test_output.pdf' generated)
[PASS] 6. Delete Note & cleanup verified
========================================
  ALL 6 TESTS PASSED! (100% SUCCESS)    
========================================
```

---

## Authors

- **FJS03** *Data Layer, Core Architecture, FTS5 Search & PDF Export*
---

## License
Copyright (c) 2026 FJS03 . All Rights Reserved. 
Usage and distribution require prior written permission from the authors. See [LICENSE](LICENSE) for details.