//
// Created by art on 28/12/16.
//

#ifndef FIRST_TEST_DBTEXTFILE_H
#define FIRST_TEST_DBTEXTFILE_H
#include <vector>
#include "../../DPInterface.h"
#include "entry.h"

class DBTextFile: public DBLibrary {
    std::string dbPath_;
    std::vector<std::string> fields_;
    std::vector<Entry> entries_;

public:
    DBTextFile(const std::string dbPath):dbPath_{dbPath} {}
    virtual ~DBTextFile() {}

    std::string fields() override;
    std::vector<std::string> get() override;
    std::string get(const std::string name, const std::string field) override;
    bool set(const std::string name, const std::string csv_values) override;
    bool set(const std::string name, const std::string field, const std::string value) override;
    bool erase(const std::string name) override;
    bool read() override;
    bool save() override;
};


#endif //FIRST_TEST_DBTEXTFILE_H
