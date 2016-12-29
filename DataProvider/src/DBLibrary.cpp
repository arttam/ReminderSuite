//
// Created by art on 28/12/16.
//
#include "DBLibrary.h"
#include "DBTextFile/DBTextFile.h"

extern "C" DBLibrary *create(const std::string type, const std::string dbPath) {
    if (type == "file") {
        return new DBTextFile(dbPath);
    }

    return nullptr;
}

extern "C" void destroy(DBLibrary* dbl) {
    delete dbl;
}
