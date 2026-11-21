#pragma once

class CLI {
public:
    static void run();

private:
    static void printHeader();
    static void handleCreate();
    static void handleList();
    static void handleView();
    static void handleSearch();
    static void handleDelete();
    static void handlePdfExport();
};