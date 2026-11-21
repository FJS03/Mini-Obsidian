cmake_minimum_required(VERSION 3.15)
project(MiniObsidian VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(SQLITE_ENABLE_FTS5)

include_directories(include)

add_library(sqlite3 STATIC src/sqlite3.c)

add_executable(mini_obsidian
    main.cpp
    src/FileIO.cpp
    src/NoteStore.cpp
    src/CLI.cpp
    src/PreviewEngine.cpp
    src/PdfExport.cpp
)
target_link_libraries(mini_obsidian PRIVATE sqlite3)
add_executable(run_tests
    tests/test_flow.cpp
    src/FileIO.cpp
    src/NoteStore.cpp
    src/PreviewEngine.cpp
    src/PdfExport.cpp
)
target_link_libraries(run_tests PRIVATE sqlite3)